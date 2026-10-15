# macOS Audio Unit

Build with `-DTINYFREAKY_BUILD_AU=ON -DAUDIOUNIT_SDK_ROOT=/path/to/AudioUnitSDK`, using Apple AudioUnitSDK revision `53a9a2008aae7fb1b0a9f093dd523b9b12f6c0d9`. The component embeds its own VST3 instrument and interface. Install the entire `Tinyfreaky.component` bundle in `~/Library/Audio/Plug-Ins/Components`; a separate VST3 installation is unnecessary. The Mac workflow verifies Intel/Apple Silicon slices, the ad-hoc signature and Apple `auval -v aumu TyFk MgSt` before publishing. Builds are not Apple-notarized.
