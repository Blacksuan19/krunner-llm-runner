#!/usr/bin/env bash

set -euo pipefail

build_dir="${BUILD_DIR:-build}"
prefix="${CMAKE_INSTALL_PREFIX:-/usr}"
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if [[ "$build_dir" != /* ]]; then
    build_dir="${script_dir}/${build_dir}"
fi

BUILD_DONE_HINT=0 "${script_dir}/build.sh"

install_cmd=(cmake --install "$build_dir")
if [[ "$prefix" == /usr* || "$prefix" == /opt* ]]; then
    install_cmd=(sudo "${install_cmd[@]}")
fi

"${install_cmd[@]}"

kbuildsycoca6

if command -v kquitapp6 >/dev/null 2>&1; then
    kquitapp6 krunner >/dev/null 2>&1 || true
fi

echo
echo "Installed. Start KRunner again with: krunner &"
