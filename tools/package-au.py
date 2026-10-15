#!/usr/bin/env python3
"""Package the self-contained AU component without losing binary permissions."""
from pathlib import Path
import argparse,zipfile
p=argparse.ArgumentParser();p.add_argument('--apple-sdk',type=Path,required=True);a=p.parse_args();r=Path(__file__).resolve().parents[1];bundle=r/'desktop/build-au/AU/Tinyfreaky.component';assert bundle.is_dir()
assert (bundle/'Contents/Resources/plugin.vst3/Contents/MacOS/Tinyfreaky').is_file()
out=r/'dist';out.mkdir(exist_ok=True)
with zipfile.ZipFile(out/'Tinyfreaky-0.3.6-macOS-AU.zip','w',zipfile.ZIP_DEFLATED) as z:
 for f in bundle.rglob('*'):
  if f.is_file():z.write(f,str(f.relative_to(bundle.parent)))
 z.writestr('INSTALL.txt','Tinyfreaky 0.3.6 Audio Unit (AUv2)\n\nUniversal Intel + Apple Silicon build. Copy the entire Tinyfreaky.component into ~/Library/Audio/Plug-Ins/Components and restart your AU host. The component contains its instrument and interface; no separate VST3 installation is required.\n\nAd-hoc signed, not Apple-notarized. Tested using Apple auval and desktop audio/state smoke tests. Six engine families remain unavailable; sequence/sample payloads are not fully supported.\n\nSource: https://github.com/MagicStino/TinyFreaky\n')
 for f in [r/'LICENSE',r/'THIRD_PARTY_NOTICES.md']:z.write(f,f.name)
 z.write(a.apple_sdk/'LICENSE.txt','APPLE-AUDIOUNIT-SDK-LICENSE.txt')
print(out/'Tinyfreaky-0.3.6-macOS-AU.zip')
