#!/usr/bin/env bash
# Force ARMv7 hard-float build; run from any working directory.
set -euo pipefail
PROJECT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UPSTREAM="${MPC_VST_TOOLS:-$PROJECT/vendor/mpc-vst-plugins}"
PYTHON="${PYTHON:-python3}"
IMAGE="${FORCE_BUILD_IMAGE:-tinyfreaky-force-cross:bullseye}"
PIN=3167a8df46c085f41277537eebaba4b8d6530ea0
if [[ ! -f "$UPSTREAM/tools/gen_vst.py" ]]; then
  echo "Set MPC_VST_TOOLS to mpc-vst-plugins checkout at $PIN" >&2; exit 1
fi
UPSTREAM="$(cd "$UPSTREAM" && pwd)"
if [[ -d "$UPSTREAM/.git" ]] && [[ "$(git -C "$UPSTREAM" rev-parse HEAD)" != "$PIN" ]]; then
  echo "Unexpected mpc-vst-plugins revision; required $PIN" >&2; exit 1
fi
"$PYTHON" -c 'import PIL' || { echo 'Python Pillow is required for skin generation.' >&2; exit 1; }
mkdir -p "$PROJECT/vst/build"
# Always begin from the pinned original, even when the checkout has local analysis patches.
rm -rf "$PROJECT/vst/build/wrapper"
git -C "$UPSTREAM" archive "$PIN" wrapper | tar -x -C "$PROJECT/vst/build"
patch -s -d "$PROJECT/vst/build" -p1 < "$PROJECT/tools/wrapper-nudge.patch"
"${HOST_CC:-cc}" -O2 -I"$UPSTREAM/tools/vendor/force-shadow/tools" \
  -o "$PROJECT/vst/build/shadow_art" "$UPSTREAM/tools/shadow_art.c" -lm
"$PYTHON" "$UPSTREAM/tools/gen_vst.py" "$PROJECT/vst/vst.json"
docker build --platform linux/amd64 -t "$IMAGE" -f "$PROJECT/tools/Dockerfile.force" "$PROJECT/tools"
"$PYTHON" - "$PROJECT" "$UPSTREAM" "$IMAGE" <<'PY'
import json, os, pathlib, re, subprocess, sys
project, upstream, image = map(str, sys.argv[1:])
cfg = json.loads(pathlib.Path(project, 'vst/vst.json').read_text())
build = cfg.get('build', {})
root = pathlib.Path(project, 'vst', build.get('root', '.')).resolve()
if root != pathlib.Path(project).resolve():
    raise SystemExit('build.root must resolve to the project root')
sources = build.get('sources', [])
if not sources or any(not s.endswith('.c') for s in sources):
    raise SystemExit('This helper currently supports C sources only')
output = '/b/vst/build/' + cfg['so']
base = ['docker', 'run', '--rm', '--platform', 'linux/amd64', '-u', f'{os.getuid()}:{os.getgid()}',
        '-v', project + ':/b', '-v', upstream + ':/upstream:ro', '-w', '/b', image]
command = ['arm-linux-gnueabihf-gcc', '-O2', '-Wall', '-Wextra', '-Wno-unused-parameter',
           '-fPIC', '-shared', '-fvisibility=hidden', '-std=gnu11',
           '-Ivst/build', '-Ivst/build/wrapper']
command += build.get('cflags', []) + build.get('cflags_arm', [])
command += sources + ['/b/vst/build/wrapper/vst2_wrap.c']
command += build.get('libs', ['-lm']) + ['-lpthread', '-Wl,--no-undefined', '-o', output]
subprocess.run(base + command, check=True)
subprocess.run(base + ['arm-linux-gnueabihf-strip', output], check=True)
report = []
for options in (['-h'], ['-A'], ['--version-info'], ['--dyn-syms', '--wide']):
    result = subprocess.run(base + ['arm-linux-gnueabihf-readelf'] + options + [output], check=True,
                            capture_output=True, text=True)
    report.append(result.stdout)
text = '\n'.join(report)
if 'Machine:                           ARM' not in text or 'hard-float ABI' not in text:
    raise SystemExit('Expected ARM hard-float ELF')
if not re.search(r'GLOBAL\s+DEFAULT\s+\d+\s+VSTPluginMain', text):
    raise SystemExit('Missing VSTPluginMain export')
versions = [tuple(map(int, v.split('.'))) for v in re.findall(r'GLIBC_([0-9.]+)', text)]
if versions and max(versions) > (2, 32):
    raise SystemExit('GLIBC requirements exceed upstream OS 2.x compatibility ceiling')
pathlib.Path(project, 'vst/build/elf-report.txt').write_text(text)
print('Verified ARM hard-float VSTPluginMain; highest GLIBC:', '.'.join(map(str, max(versions))))
print('Force ARM binary:', pathlib.Path(project, 'vst/build', cfg['so']))
print('Build and ELF checks do not replace a Force device smoke test.')
PY
