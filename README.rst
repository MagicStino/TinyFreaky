Tinyfreaky
==========

MicroFreak-style instrument for Force OpenPlugin (ARMv7 VST2) and desktop VST3.
Green, orange and white controls are grouped by the original hardware panel:
digital oscillator, filter, cycling envelope, LFO, envelope and performance.
The desktop editor and Force skin expose all 35 modulation matrix amounts.

Current status
--------------

This is a research build, not a complete firmware port. Direct firmware reverse
engineering is now permitted for this project; there is no cleanroom restriction.
The existing DSP remains independently authored and approximate. A new interface
and included factory parameters do not establish original sound equivalence.
Original analog circuitry is not contained in the firmware image.

Factory presets
---------------

All four MicroFreak factory banks shipped with the official Arturia MIDI Control
Center 1.23.0.134 package have been collected: Factory3.0, Factory6.0,
New Presets 4.0, New Presets 5.0. There are 832 populated records, 960 empty slots,
and 448 distinct decoded payloads. The original bank archive preserves every
record, including duplicate locations and empty slots.

All 448 distinct presets are compiled into the Force and desktop browser after
eight native presets, so no manual preset-folder copying is required.
334 entries have partial parameter translations; their names start with ``~``.
These use approximate DSP and do not reproduce original sequences or all routing.
114 entries require unsupported engines; their names start with ``[Unavailable]``
and selecting them mutes audio instead of playing a substitute sound.

The original banks and sample/wavetable assets are preserved under assets/factory.
The sample and wavetable archives are not yet decoded by the audio engine.
See assets/factory/index.json for every name, source bank, digest and limitation.
Factory6.0 is the bank's supplied name; it is not a claim of MicroFreak firmware 6.

Build
-----

Force: set MPC_VST_TOOLS to the pinned dependency and run tools/build-force.sh.
Skin generation requires Pillow, Playwright and Chromium. Desktop: set
VST3_SDK_ROOT to the revision in desktop/SDK_REVISION, then run
tools/build-desktop.sh. The official VST3 SDK and VSTGUI provide the custom editor.

The current build passes engine/import tests and Linux Steinberg validation.
Physical Force playback, touchscreen rendering, CPU usage and exact sound are
not verified. Windows and macOS builds of this revision still require CI.

Licenses and provenance
-----------------------

Original plugin code is MIT; SDK, VSTGUI and wrapper components retain their
licenses. Factory parameter, sample and wavetable assets originate from Arturia's
official MIDI Control Center distribution and are not relicensed as MIT.
See assets/factory/SOURCE.txt. Names identify compatibility targets; this project
is unaffiliated with Arturia, Akai, inMusic and Steinberg.
