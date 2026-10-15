#!/usr/bin/env python3
"""Hardware strip bounds and independent pitch/velocity style coverage."""
from pathlib import Path
from PIL import Image,ImageChops
import json
r=Path(__file__).resolve().parents[1]
for style in range(9):
 im=Image.open(r/f'vst/mascot-{style}.png').convert('RGBA');count=62 if style==0 else 61
 assert im.size==(112,166*count) and im.height<=12288
 frames=[im.crop((0,f*166,112,(f+1)*166)) for f in range(1,61)]
 assert not im.crop((0,0,112,166)).getchannel('A').getbbox()
 assert len({f.tobytes() for f in frames})==60
 for f in frames:
  bbox=f.getchannel('A').getbbox();assert bbox and bbox[0]>0 and bbox[2]<112 and bbox[1]>0 and bbox[3]<166,(style,bbox)
 assert any(ImageChops.difference(frames[0],f).getchannel('A').getbbox() for f in frames[1:])
params={p['key']:p for p in json.loads((r/'vst/params.json').read_text())}
assert params['screen_mascot']['max']==540
assert len(params['screen_mascot_style']['options'])==9
layout=(r/'vst/layout.conf').read_text()
for style in range(9):assert f'cx=640 cy=487 w=112 h=166 key=screen_mascot_{style}' in layout and f'when=screen_mascot_style:{style}' in layout
print('mascot: 540 distinct poses, unclipped artwork, bounded strips and style visibility PASS')
