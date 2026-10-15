TinyFreaky / MagicStino
======================

An eight-voice instrument for Force OpenPlugin, Windows/macOS/Linux VST3, and macOS Audio Unit. Sixteen engines, an output scope, readable controls, a 5 x 7 modulation matrix, arpeggiator with swing, and a 64-step sequencer.

Original TinyFreaky patch names identify 456 stable slots. Eight starter patches and 334 mapped patches are playable; 114 slots require unavailable engines and remain explicitly muted. Names are new; numeric sound settings and slot IDs are preserved. Sequence/sample payloads are not fully supported.

MagicStino branding links to https://github.com/MagicStino/microfreeze on desktop. Force has a MagicStino source page. The white line-art mascot between Master and Glide responds subtly to velocity using 17 cached poses at 10 Hz, then settles to rest.

Builds
------

Force: set MPC_VST_TOOLS to the pinned checkout in tools/build-force.sh and run that helper. Desktop: set VST3_SDK_ROOT to the recursive revision in desktop/SDK_REVISION and run tools/build-desktop.sh. AU build and installation are documented in docs/AUDIOUNIT.md. Run tools/test-engine.py and tools/test-force-host.sh for engine and host-contract checks.

Licenses
--------

Plugin code and Mutable Instruments cores are MIT. Third-party source, SDK, VSTGUI and wrapper notices remain intact. Source-bank archives and sample/wavetable assets are excluded from the instrument packages. See THIRD_PARTY_NOTICES.md.
