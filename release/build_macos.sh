#!/bin/bash
# Local equivalent of the "OSX Release" job in .github/workflows/iPDbOSX.yml:
# builds ipdb natively (macOS releases build directly on the host toolchain,
# no Docker/manylinux container like the Linux release) and leaves the
# packaged binaries under an output directory instead of uploading anything.
# Mirrors release/build_linux_docker.sh's flags/structure where it makes
# sense for a native (non-containerized) build.
#
# Usage: release/build_macos.sh [--arch amd64|arm64] [--skip-tests] [--no-ccache] [--clean] [--output-dir DIR] [--version vX.Y.Z]
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCH=""
SKIP_TESTS=0
USE_CCACHE=1
CLEAN=0
OUTPUT_DIR=""
OVERRIDE_GIT_DESCRIBE="${OVERRIDE_GIT_DESCRIBE:-}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --arch) ARCH="$2"; shift 2 ;;
    --skip-tests) SKIP_TESTS=1; shift ;;
    --no-ccache) USE_CCACHE=0; shift ;;
    --clean) CLEAN=1; shift ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --version) OVERRIDE_GIT_DESCRIBE="$2"; shift 2 ;;
    *) echo "Unknown option: $1" >&2; exit 1 ;;
  esac
done

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "This script builds natively and must be run on macOS." >&2
  exit 1
fi

HOST_ARCH="$(uname -m)"
case "$HOST_ARCH" in
  x86_64) HOST_ARCH_NORM=amd64 ;;
  arm64) HOST_ARCH_NORM=arm64 ;;
  *) HOST_ARCH_NORM="$HOST_ARCH" ;;
esac
ARCH="${ARCH:-$HOST_ARCH_NORM}"

case "$ARCH" in
  amd64) OSX_ARCH=x86_64 ;;
  arm64) OSX_ARCH=arm64 ;;
  *) echo "Unsupported --arch '$ARCH' (expected amd64 or arm64)" >&2; exit 1 ;;
esac

OUTPUT_DIR="${OUTPUT_DIR:-$REPO_ROOT/release/dist/$ARCH}"
mkdir -p "$OUTPUT_DIR"

if [[ "$CLEAN" == "1" ]]; then
  rm -rf "$REPO_ROOT/build/release"
fi

command -v brew >/dev/null 2>&1 || { echo "Homebrew not found; required for the ninja/openssl dependencies (https://brew.sh)" >&2; exit 1; }
command -v ninja >/dev/null 2>&1 || { echo "ninja not found; install with 'brew install ninja'" >&2; exit 1; }
brew --prefix openssl@3 >/dev/null 2>&1 || { echo "openssl@3 not found; install with 'brew install openssl'" >&2; exit 1; }

# CMake's static-OpenSSL/curl detection that release/build_linux_docker.sh
# needs on the manylinux container doesn't apply here: macOS's system
# libcurl + the brew-installed OpenSSL below are picked up directly, so
# there's no --static-curl equivalent for this script.
OPENSSL_PREFIX="$(brew --prefix openssl@3)"
export LDFLAGS="-L${OPENSSL_PREFIX}/lib"
export CPPFLAGS="-I${OPENSSL_PREFIX}/include"
export PKG_CONFIG_PATH="${OPENSSL_PREFIX}/lib/pkgconfig"

export EXTENSION_CONFIGS="$REPO_ROOT/.github/config/bundled_extensions.cmake"
export ENABLE_EXTENSION_AUTOLOADING=1
export ENABLE_EXTENSION_AUTOINSTALL=1
export GEN=ninja
export ENABLE_LLM_API=1
export OVERRIDE_GIT_DESCRIBE
export CMAKE_VARS="-DCMAKE_OSX_ARCHITECTURES=$OSX_ARCH"

if [[ "$USE_CCACHE" == "1" ]]; then
  command -v ccache >/dev/null 2>&1 || { echo "ccache not found; install with 'brew install ccache', or pass --no-ccache" >&2; exit 1; }
  export CMAKE_VARS="$CMAKE_VARS -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
fi

echo "$CMAKE_VARS"
make -C "$REPO_ROOT"

echo "Build finished, packaging outputs into $OUTPUT_DIR"

if [[ "$ARCH" == "$HOST_ARCH_NORM" ]]; then
  "$REPO_ROOT/build/release/ipdb" -c "PRAGMA platform;"
else
  echo "Skipping local binary checks: built for $ARCH, host is $HOST_ARCH_NORM"
fi

python3 "$REPO_ROOT/scripts/amalgamation.py"
zip -j "$OUTPUT_DIR/ipdb-cli-macos-$ARCH.zip" "$REPO_ROOT/build/release/ipdb"
zip -j "$OUTPUT_DIR/libduckdb-macos-$ARCH.zip" "$REPO_ROOT"/build/release/src/libduckdb*.* "$REPO_ROOT/src/amalgamation/duckdb.hpp" "$REPO_ROOT/src/include/duckdb.h"

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
