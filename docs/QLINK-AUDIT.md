# Q-Link automation audit — 2026-11-04

The previous pinned wrapper requests audioMasterUpdateDisplay after every host parameter write. This rebuilds the MPC automation screen and interrupts Q-Links (TinyFreaky issue #2).

The build now pins open-mpc/mpc-vst-plugins at bf1054ce08460cf42b10f96985e73141d64a71af, which opts into refresh_display only for parameters that change unpolled labels. Parameter order is locked in vst/params.lock. No catalog PR or catalog workflow is added during the announced migration pause.

The osc_type selector requests refresh_display because it changes the Wave/Timbre/Shape labels. The continuous controls do not. Restored the CPU optimizations from the published v0.4.1 source archive; main previously lacked them. Existing wrapper-nudge.patch still applies. tools/test-force-host.sh runs upstream tools/test_port.sh against this exact wrapper plus the local nudge patch.

Old workflow publication steps were hardcoded to obsolete versions; builds now produce review artifacts without overwriting a published release. Published manifests and ZIP files remain unchanged. Rebuild and package a new release after device validation.

## Physical validation still required

Use a saved copy of a project. On each supported Force/MPC Gen1 firmware, open Track Edit → Automation and sweep continuous and option parameters with Q-Links while stopped and playing. Confirm no jumps/rebuilds, record and replay automation, save/reload, switch presets and STOP/PLAY without resetting parameters. For TinyFreaky, switch Model and check all dependent labels; check preset nudges and drags. For PulyTek, check VU animation and preset redraws under audio. Check CPU/audio dropouts with several instances. Host tests and ELF validation do not establish device behavior or Gen2 support.
