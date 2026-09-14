#!/bin/bash
# Local equivalent of the "Linux CLI" job in .github/workflows/LinuxRelease.yml:
# builds ipdb inside the same manylinux_2_28 container CI uses, and leaves the
# packaged binaries under an output directory instead of uploading anything.
#
# Usage: release/build_linux_docker.sh [--arch amd64|arm64] [--skip-tests] [--no-ccache] [--clean] [--output-dir DIR] [--static-curl] [--version vX.Y.Z]
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCH=""
SKIP_TESTS=0
USE_CCACHE=1
CLEAN=0
OUTPUT_DIR=""
STATIC_CURL=0
OVERRIDE_GIT_DESCRIBE="${OVERRIDE_GIT_DESCRIBE:-}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --arch) ARCH="$2"; shift 2 ;;
    --skip-tests) SKIP_TESTS=1; shift ;;
    --no-ccache) USE_CCACHE=0; shift ;;
    --clean) CLEAN=1; shift ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --static-curl) STATIC_CURL=1; shift ;;
    --version) OVERRIDE_GIT_DESCRIBE="$2"; shift 2 ;;
    *) echo "Unknown option: $1" >&2; exit 1 ;;
  esac
done

HOST_ARCH="$(uname -m)"
case "$HOST_ARCH" in
  x86_64) HOST_ARCH_NORM=amd64 ;;
  aarch64|arm64) HOST_ARCH_NORM=arm64 ;;
  *) HOST_ARCH_NORM="$HOST_ARCH" ;;
esac
ARCH="${ARCH:-$HOST_ARCH_NORM}"

case "$ARCH" in
  amd64) IMAGE_ARCH=x86_64 ;;
  arm64) IMAGE_ARCH=aarch64 ;;
  *) echo "Unsupported --arch '$ARCH' (expected amd64 or arm64)" >&2; exit 1 ;;
esac

OUTPUT_DIR="${OUTPUT_DIR:-$REPO_ROOT/release/dist/$ARCH}"
CACHE_DIR="$REPO_ROOT/release/.docker-cache/$ARCH"
mkdir -p "$OUTPUT_DIR" "$CACHE_DIR/ccache"

if [[ "$CLEAN" == "1" ]]; then
  rm -rf "$REPO_ROOT/build/release"
fi

CCACHE_SETUP=""
EXTRA_CMAKE_VARS="-DCMAKE_EXE_LINKER_FLAGS=-ldl"
if [[ "$USE_CCACHE" == "1" ]]; then
  CCACHE_SETUP="yum install -y ccache"
  EXTRA_CMAKE_VARS="$EXTRA_CMAKE_VARS -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
fi

# By default curl and OpenSSL are picked up dynamically from the manylinux
# container's own libcurl-devel/openssl-devel packages. Pass --static-curl to
# instead build (and cache) a static libcurl linked against a static OpenSSL,
# e.g. to produce a binary with no runtime dependency on the container's
# libcurl.so/libssl.so sonames. OpenSSL is switched to static in lockstep with
# curl (rather than via its own flag): a dynamic-curl + static-OpenSSL mix
# fails to link, because CMake's static-OpenSSL detection also requires a
# static libz to resolve libcrypto's transitive deps. The distro's zlib-static
# package can't be used for that either: zlib's own configure compiles the
# static archive without -fPIC by default, so it can't be embedded into the
# httpfs_loadable_extension shared object -- we build our own instead (see
# build_static_zlib.sh and build_static_curl.sh, both of which force
# CFLAGS=-fPIC for exactly this reason -- OpenSSL is the exception, as its
# own build already compiles everything PIC).
CURL_VOLUME_MOUNT=""
ZLIB_VOLUME_MOUNT=""
CURL_BUILD_STEP=""
STATIC_OPENSSL_ENV=""
STATIC_OPENSSL_STEP=""
if [[ "$STATIC_CURL" == "1" ]]; then
  mkdir -p "$CACHE_DIR/curl-static" "$CACHE_DIR/zlib-static"
  CURL_VOLUME_MOUNT="-v$CACHE_DIR/curl-static:/opt/curl-static"
  ZLIB_VOLUME_MOUNT="-v$CACHE_DIR/zlib-static:/opt/zlib-static"
  EXTRA_CMAKE_VARS="$EXTRA_CMAKE_VARS -DCURL_INCLUDE_DIR=/opt/curl-static/include -DCURL_LIBRARY=/opt/curl-static/lib/libcurl.a"
  EXTRA_CMAKE_VARS="$EXTRA_CMAKE_VARS -DZLIB_INCLUDE_DIR=/opt/zlib-static/include -DZLIB_LIBRARY=/opt/zlib-static/lib/libz.a"
  CURL_BUILD_STEP="
    if [ ! -f /opt/zlib-static/lib/libz.a ]; then
      bash '$REPO_ROOT/scripts/build_static_zlib.sh' /opt/zlib-static
    fi
    if [ ! -f /opt/curl-static/lib/libcurl.a ]; then
      bash '$REPO_ROOT/scripts/build_static_curl.sh' /opt/curl-static
    fi
  "
  STATIC_OPENSSL_ENV="-e STATIC_OPENSSL=1"
  STATIC_OPENSSL_STEP="
    yum install -y https://repo.almalinux.org/almalinux/8/devel/$IMAGE_ARCH/os/Packages/openssl-static-1.1.1k-17.el8_6.$IMAGE_ARCH.rpm
  "
fi

docker run --rm                                                       \
  -v"$REPO_ROOT":"$REPO_ROOT"                                         \
  $CURL_VOLUME_MOUNT                                                  \
  $ZLIB_VOLUME_MOUNT                                                  \
  -v"$CACHE_DIR/ccache":/root/.ccache                                 \
  -e CMAKE_BUILD_PARALLEL_LEVEL="$(nproc)"                            \
  -e OVERRIDE_GIT_DESCRIBE="${OVERRIDE_GIT_DESCRIBE:-}"                \
  -e EXTENSION_CONFIGS="$REPO_ROOT/.github/config/bundled_extensions.cmake" \
  -e ENABLE_EXTENSION_AUTOLOADING=1                                    \
  -e ENABLE_EXTENSION_AUTOINSTALL=1                                    \
  -e BUILD_BENCHMARK=1                                                 \
  -e FORCE_WARN_UNUSED=1                                               \
  -e ENABLE_LLM_API=1                                                  \
  $STATIC_OPENSSL_ENV                                                  \
  -e EXTRA_CMAKE_VARIABLES="$EXTRA_CMAKE_VARS"                         \
  quay.io/pypa/manylinux_2_28_"$IMAGE_ARCH"                            \
  bash -c "
    set -e
    # The container runs as root, so anything it writes under the bind-mounted
    # repo (build/, the docker-cache dirs) would otherwise end up root-owned
    # on the host. Chown it all back to the host user on the way out, even if
    # the build itself fails, so a plain 'rm -rf build' keeps working.
    trap 'chown -R $(id -u):$(id -g) \"$REPO_ROOT/build\" \"$CACHE_DIR\" 2>/dev/null || true' EXIT
    yum install -y perl-IPC-Cmd gcc-toolset-12 gcc-toolset-12-gcc-c++ openssl-devel libcurl-devel
    $CCACHE_SETUP
    $STATIC_OPENSSL_STEP
    source /opt/rh/gcc-toolset-12/enable
    export CC=gcc
    export CXX=g++
    $CURL_BUILD_STEP
    git config --global --add safe.directory '$REPO_ROOT'
    make -C '$REPO_ROOT'
  "

echo "Build finished, packaging outputs into $OUTPUT_DIR"

if [[ "$ARCH" == "$HOST_ARCH_NORM" ]]; then
  "$REPO_ROOT/build/release/ipdb" -c "PRAGMA platform;"
else
  echo "Skipping local binary checks: built for $ARCH, host is $HOST_ARCH_NORM"
fi

python3 "$REPO_ROOT/scripts/amalgamation.py"
zip -j "$OUTPUT_DIR/ipdb-cli-linux-$ARCH.zip" "$REPO_ROOT/build/release/ipdb"
gzip -9 -k -n -c "$REPO_ROOT/build/release/ipdb" > "$OUTPUT_DIR/ipdb-cli-linux-$ARCH.gz"
zip -j "$OUTPUT_DIR/libduckdb-linux-$ARCH.zip" "$REPO_ROOT"/build/release/src/libduckdb*.* "$REPO_ROOT/src/amalgamation/duckdb.hpp" "$REPO_ROOT/src/include/duckdb.h"

if [[ "$SKIP_TESTS" == "1" ]]; then
  echo "Skipping tests (--skip-tests)"
elif [[ "$ARCH" != "$HOST_ARCH_NORM" ]]; then
  echo "Skipping tests: built for $ARCH, host is $HOST_ARCH_NORM"
else
  python3 -m pip install --quiet pytest
  python3 "$REPO_ROOT/scripts/run_tests_one_by_one.py" "$REPO_ROOT/build/release/test/unittest" "*" --time_execution
  "$REPO_ROOT/build/release/test/unittest" --select-tag release
  python3 -m pytest "$REPO_ROOT/tools/shell/tests" --shell-binary "$REPO_ROOT/build/release/ipdb"
  "$REPO_ROOT/build/release/benchmark/benchmark_runner" "$REPO_ROOT/benchmark/micro/update/update_with_join.benchmark"
  "$REPO_ROOT/build/release/ipdb" -c "COPY (SELECT 42) TO '/dev/stdout' (FORMAT PARQUET)" | cat
fi

echo "Done. Packaged artifacts:"
ls -la "$OUTPUT_DIR"
