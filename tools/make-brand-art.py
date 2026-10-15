#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original white vector mascot, sampled once for both native editor and Force.
17 cached poses, maximum head rotation 1.2 degrees and body travel 0.8 units.
No fonts, drawing, image loading or allocations run in the audio thread.
"""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json,re,math
r=Path(__file__).resolve().parents[1]
def flatten(s):
 t=re.findall(r'[MLCZ]|-?\d+(?:\.\d+)?',s);i=0;points=[];first=None
 while i<len(t):
  op=t[i];i+=1
  if op in ('M','L'):
   point=(float(t[i]),float(t[i+1]));i+=2;points.append(point)
   if first is None:first=point
  elif op=='C':
   a=points[-1];b=tuple(map(float,t[i:i+2]));c=tuple(map(float,t[i+2:i+4]));e=tuple(map(float,t[i+4:i+6]));i+=6
   for n in range(1,13):
    u=n/12;v=1-u;points.append((v**3*a[0]+3*v*v*u*b[0]+3*v*u*u*c[0]+u**3*e[0],v**3*a[1]+3*v*v*u*b[1]+3*v*u*u*c[1]+u**3*e[1]))
  elif op=='Z':points.append(first)
 return points
# Each M starts a separate continuous polyline.
curves=[]
for part,path in json.loads((r/'assets/mascot-paths.json').read_text()):
 for chunk in re.findall(r'M[^M]+',path):curves.append((part,flatten(chunk)))
def pose(points,part,amount):
 out=[]
 for x,y in points:
  if part=='head':
   angle=amount*math.radians(1.2);dx=x-49;dy=y-73;x=49+dx*math.cos(angle)-dy*math.sin(angle);y=73+dx*math.sin(angle)+dy*math.cos(angle)+amount*.35
  elif part=='hip':x+=amount*.8;y+=amount*.25
  else:x+=amount*.35*max(0,min(1,(y-76)/74));y+=amount*.3
  out.append((x,y))
 return out
inc=[]
for frame in range(17):
 amount=(frame-8)/8
 for part,points in curves:
  p=pose(points,part,amount)
  for a,b in zip(p,p[1:]):inc.append('{%d,%.3f,%.3f,%.3f,%.3f},'%(frame,*a,*b))
(r/'desktop/mascot.inc').write_text('\n'.join(inc)+'\n')
w,h,scale=76,166,3
film=Image.new('RGBA',(w,h*17))
for frame in range(17):
 im=Image.new('RGBA',(w*scale,h*scale));d=ImageDraw.Draw(im)
 for part,points in curves:
  p=pose(points,part,(frame-8)/8);d.line([(int((x*.7+3)*scale),int((y*.72+5)*scale)) for x,y in p],fill='#f2efdf',width=3,joint='curve')
 im=im.resize((w,h),Image.Resampling.LANCZOS);film.paste(im,(0,h*frame))
film.save(r/'vst/mascot-strip.png')
# Slanted serif wordmark, small orange crossbar; font license is system DejaVu.
font='/usr/share/fonts/truetype/dejavu/DejaVuSerifCondensed-BoldItalic.ttf'
im=Image.new('RGBA',(600,104));d=ImageDraw.Draw(im);f=ImageFont.truetype(font,70)
d.text((5,-2),'TinyFreaky',font=f,fill='#f2efdf',stroke_width=0)
d.line((16,87,112,87),fill='#f18a3c',width=4)
im=im.resize((300,52),Image.Resampling.LANCZOS);im.save(r/'desktop/resources/wordmark.png');im.save(r/'assets/wordmark.png')
print('17 bounded mascot poses; shared line segments and wordmark generated')
