# Desktop VST3

This wrapper builds the same independent Tinyfreaky engine using Steinberg's official MIT VST3 SDK. The SDK revision is pinned in `SDK_REVISION`; its license is included. It does not incorporate Arturia firmware.

## Build

Clone `https://github.com/steinbergmedia/vst3sdk.git` recursively, check out the revision in `desktop/SDK_REVISION`, and update its recursive submodules. Set `VST3_SDK_ROOT` to that checkout and run `tools/build-desktop.sh`. CMake 3.25+, a C/C++ compiler, Python 3, and platform development libraries are required. The supplied `tools/Dockerfile.desktop` supplies Linux dependencies. CI builds Linux x86_64, Windows x64, and macOS universal binaries.

The bundle is generated at `desktop/build/VST3/Release/Tinyfreaky.vst3`. Run `ctest --test-dir desktop/build -C Release --output-on-failure` for the desktop smoke test. The SDK validator also runs automatically when the plugin is linked.

## Install and use

Copy the entire `.vst3` bundle into the VST3 folder scanned by your DAW. Common per-user destinations are `~/.vst3` on Linux, `~/Library/Audio/Plug-Ins/VST3` on macOS, and the DAW's configured VST3 directory on Windows. Windows system installations conventionally use `C:\Program Files\Common Files\VST3`.

The DAW provides the generic parameter editor. All numeric engine parameters are exposed, including named oscillator and other enumeration choices. Preset slots remain 0–511 for stable automation; unavailable slots are labelled. The initial eight independent presets are available without an external bank.

To use a converted Tinyfreaky preset bank, set the `TINYFREAKY_PRESETS` environment variable to its directory before launching the DAW. Processor and controller load the same directory when instantiated. This setting points to converted engine presets, not directly to an MCC `.mfprojz` archive; use the repository's preset importer first. Reload the plugin after changing the directory.

The wrapper accepts 16-channel note events and poly pressure. MIDI CC, channel pressure, and pitch bend address global instrument controls. Host automation and note events are dispatched at their sample offsets. Processing uses 32-bit floating-point stereo output; the engine runs at 44.1 kHz internally. Other host rates use persistent linear interpolation, preserving state across blocks. This is a basic streaming converter, not a bandlimited mastering resampler. The 44.1 kHz path bypasses conversion.

Host state preserves exact engine state and parameter values. Loading a preset refreshes host automation values. The wrapper is separate from the Force ARM VST2 build.

## Validation

Final engine build (16 supported oscillator choices, preset range 0–511): Linux x86_64 official SDK validator passed 47 tests with zero failures. Additional smoke checks pass at 44.1, 48 and 96 kHz, verify finite audible output, block-partition invariance, note timing at 44.1 kHz and host-state restoration. The same checks also pass at the SDK stress rates 1234.568 and 1234567.8 Hz. Windows and macOS CI validation is pending for this revision. Distribution bundles are unsigned.
