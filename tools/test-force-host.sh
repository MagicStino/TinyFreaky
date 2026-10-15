#!/usr/bin/env bash
# Run the unchanged upstream VST ABI test against our reproducibly patched wrapper.
set -euo pipefail
PROJECT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UPSTREAM="${MPC_VST_TOOLS:-$PROJECT/vendor/mpc-vst-plugins}"
PIN=3167a8df46c085f41277537eebaba4b8d6530ea0
mkdir -p "$PROJECT/vst/build"
TESTTOOLS="$(mktemp -d "$PROJECT/vst/build/abi-tools.XXXXXX")"
trap 'rm -rf "$TESTTOOLS"' EXIT
git -C "$UPSTREAM" archive "$PIN" tools wrapper adapters | tar -x -C "$TESTTOOLS"
patch -s -d "$TESTTOOLS" -p1 < "$PROJECT/tools/wrapper-nudge.patch"
"$TESTTOOLS/tools/test_port.sh" "$PROJECT/vst/vst.json"
