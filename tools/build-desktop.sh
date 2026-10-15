#!/usr/bin/env bash
set -euo pipefail
PROJECT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
: "${VST3_SDK_ROOT:?Set VST3_SDK_ROOT to recursive official vst3sdk checkout}"
PIN="$(cat "$PROJECT/desktop/SDK_REVISION")"
[[ "$(git -C "$VST3_SDK_ROOT" rev-parse HEAD)" == "$PIN" ]] || { echo 'SDK revision mismatch' >&2; exit 1; }
python3 "$PROJECT/tools/gen-desktop-params.py"
cmake -S "$PROJECT/desktop" -B "$PROJECT/desktop/build" -DCMAKE_BUILD_TYPE=Release -DVST3_SDK_ROOT="$VST3_SDK_ROOT" "$@"
cmake --build "$PROJECT/desktop/build" --config Release --parallel 2
