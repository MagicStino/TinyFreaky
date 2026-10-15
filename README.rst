Microfreeze
===========

Independent synthesizer prototype for Akai Force OpenPlugin (ARMv7 VST2) and desktop hosts (VST3).

Status
------

This is an experimental cleanroom implementation, not the Arturia firmware running in a plugin. Sixteen oscillator prototypes, four-voice paraphony, a shared multimode filter, envelopes, LFO, MIDI expression and a 5 x 7 modulation matrix are implemented. Original sound equivalence has not been established. Read MODEL_COVERAGE.txt before using it for an existing patch.

User wavetable, sample/granular and vocoder models are unavailable and are explicitly rejected. Original sequences and some modulation assignments are not implemented. The panel inventory documents the target feature set; it does not imply every documented feature is implemented.

Force
-----

Use the ARMv7 portable package with Force OpenPlugin on Force 3.9.1. The package contains the library, touchscreen pages, Q-Link mapping and upstream installer. Hardware playback and CPU use still need testing. Follow the install instructions inside the ZIP. The desktop VST3 is a separate binary.

Eight independently created presets are included. Save the plugin in a Force project to preserve its parameter state. User .mfz banks are read from the plugin's presets directory when the instrument is instantiated; reopen the instrument after adding files.

Original presets
----------------

tools/import-presets.py reads legacy .mbp/.mfp and ZIP .mfprojz/.mfpz containers into a preservation report. It maps understood controls and retains unknown records. It does not claim matching sound, sequences or samples. The V5 engine order still needs fixture verification.

Run ``python3 tools/import-presets.py input.mfprojz converted`` for reports. Only ``--allow-partial`` explicitly emits experimental .mfz patches for supported models. Inspect every report. Unsupported models are not emitted as playable replacements. No original bank or firmware asset is distributed here.

Desktop
-------

The desktop version uses Steinberg's official VST3 SDK. Linux, Windows and macOS require separate builds. The host's generic parameter editor is used; the Force touchscreen skin is specific to Force. See desktop/ and the desktop build workflow. Engine processing is 44.1 kHz internally; the desktop wrapper converts other host rates.

Build and test
--------------

Force build: set MPC_VST_TOOLS to the pinned upstream checkout and run tools/build-force.sh. See docs/FORCE-BUILD.md. Desktop: set VST3_SDK_ROOT to the recursive checkout at desktop/SDK_REVISION and run tools/build-desktop.sh.

Engine tests: ``cc -std=c99 -Wall -Wextra -Werror -Idesktop src/engine.c tests/engine_test.c -lm -o engine-test`` followed by ``./engine-test``. Importer tests: ``python3 tests/test_import.py``.

Cleanroom boundary
------------------

The engine was written by an implementation worker that received only the public-manual control specification and the MIT host ABI. It was instructed not to inspect the firmware, disassembly, recovered DSP, original tables or other synth implementations. Research and implementation were separated. This is a recorded workflow boundary, not an independently audited cleanroom certification.

Public control and file-format specifications are in docs/. Original firmware, disassembly, factory banks and extracted audio data are excluded. Firmware inventory observations are research evidence rather than a recovered audio implementation. Full behavioral reverse engineering remains incomplete.

License
-------

Independent code is MIT. Steinberg SDK components and the Force wrapper retain their respective MIT notices. Product and model names identify compatibility targets and documented controls; this project is unaffiliated with Arturia, Akai, inMusic or Steinberg.
