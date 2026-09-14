#!/bin/bash
# Builds a static libz compiled with -fPIC, so it can be linked into duckdb's
# loadable (shared object) extensions.
#
# The distro's zlib-static package is unusable for this: zlib's own configure
# script intentionally compiles two separate object sets (PIC for libz.so,
# non-PIC for libz.a) since it doesn't reuse shared-lib objects for the static
# archive the way OpenSSL/curl do. Linking that non-PIC libz.a into a shared
# object fails with "relocation ... can not be used when making a shared
# object; recompile with -fPIC". Building our own with CFLAGS=-fPIC sidesteps
# that.
#
# Usage: build_static_zlib.sh <install-prefix>
# Requires: a C compiler on PATH.
set -euo pipefail

ZLIB_VERSION="${ZLIB_VERSION:-1.3.1}"
PREFIX="${1:?usage: build_static_zlib.sh <install-prefix>}"

WORKDIR=$(mktemp -d)
trap 'rm -rf "$WORKDIR"' EXIT
cd "$WORKDIR"

# zlib.net's root only serves the current latest release; /fossils/ keeps a
# permanent copy of every past release, so this stays reproducible even after
# ZLIB_VERSION falls behind upstream's latest.
curl -fsSL "https://zlib.net/fossils/zlib-${ZLIB_VERSION}.tar.gz" -o zlib.tar.gz
tar xzf zlib.tar.gz
cd "zlib-${ZLIB_VERSION}"

CFLAGS="-fPIC" ./configure --prefix="$PREFIX" --static

make -j"$(nproc)"
make install
