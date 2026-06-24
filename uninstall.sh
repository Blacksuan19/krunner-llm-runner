#!/usr/bin/env bash

set -euo pipefail

prefix="${CMAKE_INSTALL_PREFIX:-/usr}"
build_dir="${BUILD_DIR:-build}"
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
qt_plugin_dir="${prefix}/lib/qt6/plugins/kf6/krunner"
generic_plugin_dir="${prefix}/lib/plugins/kf6/krunner"

files=()

if [[ "$build_dir" != /* ]]; then
    build_dir="${script_dir}/${build_dir}"
fi

if [[ -f "${build_dir}/install_manifest.txt" ]]; then
    mapfile -t files <"${build_dir}/install_manifest.txt"
fi

files+=(
    "${prefix}/share/krunner/dbusplugins/plasma-runner-llm.desktop"
    "${prefix}/share/krunner/kcms/kcm_krunner_llm.desktop"
    "${qt_plugin_dir}/krunner_llm.so"
    "${qt_plugin_dir}/kcms/kcm_krunner_llm.so"
    "${generic_plugin_dir}/krunner_llm.so"
    "${generic_plugin_dir}/kcms/kcm_krunner_llm.so"
)

remove_cmd=(rm -f "${files[@]}")
if [[ "$prefix" == /usr* || "$prefix" == /opt* ]]; then
    remove_cmd=(sudo "${remove_cmd[@]}")
fi

"${remove_cmd[@]}"
kbuildsycoca6

if command -v kquitapp6 >/dev/null 2>&1; then
    kquitapp6 krunner >/dev/null 2>&1 || true
fi

echo "Uninstalled. Start KRunner again with: krunner &"
