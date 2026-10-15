#!/usr/bin/env python3
"""Small shared, static filmstrips; no image generation happens while playing."""
from pathlib import Path
from PIL import Image,ImageDraw
r=Path(__file__).resolve().parents[1]/'vst'
for name,w in [('scope',10),('env',8),('lfo',8)]:
 im=Image.new('RGB',(w,56*17),'#070f15');d=ImageDraw.Draw(im)
 for frame in range(17):
  y=frame*56;d.line((0,y+28,w-1,y+28),fill='#243c46')
  if name=='scope':p=y+52-round(frame*48/16);d.line((0,p,w-1,p),fill='#d9f8ff',width=3)
  elif name=='env':d.rectangle((1,y+54-round(frame*52/16),w-2,y+54),fill='#b8db91')
  else:p=y+52-round(frame*48/16);d.rectangle((1,p-2,w-2,p+2),fill='#f18a3c')
 im.save(r/(name+'-strip.png'))
im=Image.new('RGB',(32,32*4),'#070f15');d=ImageDraw.Draw(im)
d.rectangle((8,8,23,23),fill='#507260');d.polygon([(8,39),(25,48),(8,57)],fill='#f18a3c');d.rectangle((8,72,12,88),fill='#b8db91');d.rectangle((20,72,24,88),fill='#b8db91');d.rectangle((8,104,24,120),outline='#f18a3c',width=3);im.save(r/'state-strip.png')
