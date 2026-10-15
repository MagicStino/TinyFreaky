#!/usr/bin/env python3
"""Make zero-depth matrix cells visually quiet; nonzero routes remain orange."""
from pathlib import Path
import json
from PIL import Image
r=Path(__file__).resolve().parents[1];skin=r/'vst/build/skin/MagicStino - VST - Tinyfreaky/Plugin Skins'
d=json.loads((skin/'TUI.json').read_text())
for definition in d['pageData']['componentDefinitions']['localComponentDefinitions']:
 if not definition['key'].startswith('shKnobSide14'):continue
 for node in definition['value']['componentsData']:
  info=node['componentData']
  if info['type']!='Knob':continue
  src=skin/info['data']['filmStrip'];im=Image.open(src).convert('RGBA');size=im.width
  for frame in [62,63,64]:
   for y in range(size):
    for x in range(size):
     rr,g,b,a=im.getpixel((x,frame*size+y))
     if a and rr>150 and rr>g:im.putpixel((x,frame*size+y),(75,112,94,a))
  im.save(src)
