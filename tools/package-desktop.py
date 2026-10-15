#!/usr/bin/env python3
"""Package the tested desktop bundle with install instructions and licensing."""
import argparse,zipfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--platform',required=True);p.add_argument('--version',required=True);a=p.parse_args();r=Path(__file__).resolve().parents[1]
bundles=[p for p in (r/'desktop/build/VST3').rglob('Tinyfreaky.vst3') if p.is_dir()];assert len(bundles)==1,bundles
folder=bundles[0];out=r/'dist';out.mkdir(exist_ok=True)
install={'Windows':r'C:\Program Files\Common Files\VST3','macOS':'~/Library/Audio/Plug-Ins/VST3','Linux':'~/.vst3'}[a.platform]
with zipfile.ZipFile(out/f'Tinyfreaky-{a.version}-{a.platform}-VST3.zip','w',zipfile.ZIP_DEFLATED) as z:
 for f in folder.rglob('*'):
  if f.is_file():z.write(f,str(f.relative_to(folder.parent)))
 z.writestr('INSTALL.txt',f'Tinyfreaky {a.version} VST3 for {a.platform}\n\nCopy the entire Tinyfreaky.vst3 bundle into {install}, then rescan plug-ins in your DAW.\n\nmacOS: universal Intel/Apple Silicon build, ad-hoc signed, not Apple-notarized.\nWindows: x64 build.\n\nEight-voice paraphonic instrument. Six engine families remain unavailable; sequence/sample payloads are not fully supported.\nSource: https://github.com/MagicStino/TinyFreaky\n')
 for f in [r/'LICENSE',r/'THIRD_PARTY_NOTICES.md',r/'README.rst']:
  if f.exists():z.write(f,f.name)
print(out)
