# Force build

The build helper works from any current directory. It requires Docker, a native C compiler, Python 3 and Pillow, plus the separate `mpc-vst-plugins` tools checkout pinned at `3167a8df46c085f41277537eebaba4b8d6530ea0`.

```sh
MPC_VST_TOOLS=/absolute/path/to/mpc-vst-plugins ./tools/build-force.sh
```

Set `PYTHON=/path/to/python3` if Pillow is installed in a different Python environment. `HOST_CC` selects the native skin-artwork compiler. The default Docker image name is `tinyfreaky-force-cross:bullseye`; override `FORCE_BUILD_IMAGE` if needed.

The helper builds skin artwork locally with the upstream renderer and generates `vst/build/params.h`, the native skin, and `pluginlist-entry.xml`. The plugin is cross-compiled in an isolated amd64 Debian bullseye container with the Debian ARM hard-float compiler and glibc 2.31 sysroot. No global emulator registration or privileged container is required. The Dockerfile pins its Debian base image digest; package versions come from the bullseye repositories and are not fully hermetically pinned.

The helper checks that `vst/build/tinyfreaky.so` is ARM hard-float, exports `VSTPluginMain`, and requires no GLIBC version newer than 2.32. Complete ELF inspection is saved to `vst/build/elf-report.txt`. Generated skins and binaries are build outputs; rebuild them when parameters change.

## Verification on 2026-10-15

- Native Docker host: x86-64 Linux.
- Upstream `arm32v7/gcc:11-bullseye` image downloaded successfully but cannot execute here: `exec format error`, because ARM QEMU/binfmt is unavailable.
- The cross-compiler alternative produced a real ARMv7 EABI5 hard-float shared library with VFPv3-D16 attributes.
- The completed Tinyfreaky plugin compiled and exported `VSTPluginMain`; highest GLIBC dependency: 2.27.
- Skin generation completed without layout warnings. The upstream artwork renderer emitted one compiler warning about potentially truncated text formatting; it did not prevent rendering.

Compilation and ELF checks establish the binary architecture and link compatibility. They do not establish device CPU usage, correct playback on Force, project persistence on hardware, or sonic equivalence to MicroFreak. Device smoke testing remains required. This build process does not access or modify a Force.

## Local wrapper correction

`tools/wrapper-nudge.patch` applies to the MIT-licensed upstream wrapper at commit `3167a8df46c085f41277537eebaba4b8d6530ea0`. Its SHA-256 is `9c90ee28bcc93ff9a3d089b025e2753c4fb965b14d6fabca0870b645b385c006`. The helper exports that original wrapper from Git into `vst/build/wrapper` and patches the copy each time. A modified upstream working tree is never used as the wrapper build input. Keep the upstream Git history containing the pinned commit available, and install Git, tar and patch.

The patch fixes two related control regressions found by the unchanged upstream VST host test: a continuous drag of a long preset list was misclassified as discrete wheel ticks and could reverse, and deferred parameter-zero updates could collapse the initial drag position. It retains tick handling at clamped edges and the host STOP-toggle suppression. All original MIT notices remain intact. This changes wrapper control handling, not the instrument DSP.

The unchanged upstream `tools/test_port.sh vst/vst.json` passed on x86 with ASan and UBSan after the correction, including option selection, wheel/Q-Link stepping, long-list monotonic sweeps, 512 program names and selection, audio, state roundtrip, STOP-toggle suppression and concurrent UI/audio calls. `ASAN_OPTIONS=detect_leaks=0` was required because LeakSanitizer cannot run in this sandbox's ptrace environment; leak detection was not performed. Host-position timing was not asserted by that harness because this engine exposes no `pos_log` probe. The plugin's separate tests cover its own timing behavior.

Run the reproducible wrapper ABI check with:

```sh
MPC_VST_TOOLS=/absolute/path/to/mpc-vst-plugins ./tools/test-force-host.sh
```

Only in environments where LeakSanitizer reports that ptrace is unsupported, prefix that invocation with `ASAN_OPTIONS=detect_leaks=0`. This retains address and undefined-behavior checking, while explicitly omitting leak checking. The helper exports and patches a private copy of the pinned upstream tools, so the original checkout stays unchanged.
