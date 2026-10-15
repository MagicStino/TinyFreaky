Live display
============

Actual output scope, envelope/LFO indicators and play/wait/hold/idle symbols update at 10 Hz using fixed buffers. See docs/UPDATE-0.3.3.txt for scope scaling and hardware limits.

Eight-voice update
==================

Paraphonic mode supports eight simultaneous notes on every implemented engine. Mono remains one voice. See docs/UPDATE-0.3.2.txt for compatibility and hardware-performance limits.

Tinyfreaky
==========

An experimental synthesizer for Akai Force OpenPlugin and desktop VST3. Green, orange and white controls, a live OLED-style display, a readable 5x7 modulation matrix, arpeggiator, swing and an editable 64-step note sequencer.

Version 0.3 adds original MIT Mutable Instruments cores to eleven oscillator paths, corrects the shared bipolar Filter Amount route, imports identified factory arp controls and adds rhythm/state support. See docs/UPDATE-0.3.txt for operation and compatibility.

Factory data includes all 448 unique originals collected from the official MIDI Control Center banks, with source records retained. 334 entries use currently available models. 114 entries require missing user-table/sample/granular/vocoder engines and remain clearly unavailable. Original sequence/modulation bytes are retained but not yet decoded. Arturia sound equivalence is unverified.

Builds
------

Force: set MPC_VST_TOOLS to the pinned checkout listed in tools/build-force.sh, then run that helper. It builds the ARM instrument and skin. Run tools/test-engine.py and tools/test-force-host.sh for the native checks.

Desktop: set VST3_SDK_ROOT to desktop/SDK_REVISION with recursive submodules, then run tools/build-desktop.sh. The official Steinberg SDK and VSTGUI provide the VST3/editor.

Licenses
--------

Plugin code and the upstream Mutable Instruments sources are MIT, with full upstream headers and revision provenance retained. SDK, VSTGUI, wrapper and fonts retain their notices. Factory parameter, sample and wavetable assets originate from Arturia's official MIDI Control Center and are not relicensed as MIT; see assets/factory/SOURCE.txt. Artwork is a generated promotional illustration, not a photograph of hardware. This independent project is unaffiliated with Arturia, Akai, inMusic or Steinberg.
