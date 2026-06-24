#!/usr/bin/env bash

set -euo pipefail

build_dir="${BUILD_DIR:-build}"
prefix="${CMAKE_INSTALL_PREFIX:-/usr}"
build_type="${CMAKE_BUILD_TYPE:-Release}"
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if [[ "$build_dir" != /* ]]; then
    build_dir="${script_dir}/${build_dir}"
fi

cmake -S "$script_dir" -B "$build_dir" \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_INSTALL_PREFIX="$prefix"

cmake --build "$build_dir" --parallel

if [[ "${BUILD_DONE_HINT:-1}" != 0 ]]; then
    echo
    echo "Build complete."
    echo "Install with: ./install.sh"
fi
