#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
plugin_dir="$project_dir/build/bin"
if [[ ! -f "$plugin_dir/de/agundur/kfritz/libkfritzplugin.so" ]]; then
    echo "Build KFritz first: cmake -S . -B build && cmake --build build" >&2
    exit 1
fi

# Always load the matching branch plugin, rather than the installed version.
export QML_IMPORT_PATH="$plugin_dir${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"
exec plasmoidviewer -a "$project_dir/package" -s 480x600 "$@"
