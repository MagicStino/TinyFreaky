#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original white vector mascot, sampled once for both native editor and Force.
540 cached poses with independent pitch and velocity styles.
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
# 60 phase samples x three pitches x three velocities + neutral.
# A single 76x30046 strip stays below a 32768px texture dimension.
def pose(points,part,frame):
 if frame==0:return points
 index=frame-1;t=(index%60)/60*2*math.pi;style=index//60
 high=(style//3)*.5;low=1-high;v=[.22,.55,1][style%3]
 hip=3*(1.6*v+8.6*low*v**.4)*math.sin(t)
 torso=math.radians((2+8*high)*v*math.sin(t+.55))
 head=math.radians((4+17*v*v+5*high)*v*math.sin(t+1.1))
 bob=(.8+4.2*v*v)*v*math.sin(2*t+.7)
 def upper(x,y):
  dx=x-49;dy=y-150
  return 49+hip+dx*math.cos(torso)-dy*math.sin(torso),150+dx*math.sin(torso)+dy*math.cos(torso)
 out=[]
 for x,y in points:
  if part=='head':
   dx=x-49;dy=y-73
   x,y=upper(49+dx*math.cos(head)-dy*math.sin(head),73+dx*math.sin(head)+dy*math.cos(head)+bob)
  elif part=='hip':x+=hip*(1-max(0,min(1,(y-150)/64)))
  else:x,y=upper(x,y)
  out.append((x,y))
 return out
# Native editor deforms this small base mesh on the UI thread only.
inc=[]
for part,points in curves:
 for a,b in zip(points,points[1:]):inc.append('{%d,%.3ff,%.3ff,%.3ff,%.3ff},'%({'head':0,'body':1,'hip':2}[part],*a,*b))
(r/'desktop/mascot.inc').write_text('\n'.join(inc)+'\n')
w,h,scale=112,166,3
# Each style has 60 phases, plus blank frame zero. Stay below Akai's
# device-confirmed 12288px limit; IndexedEnabling shows just one strip.
for style in range(9):
 film=Image.new('RGBA',(w,h*(62 if style==0 else 61)))
 for phase in range(60):
  frame=1+style*60+phase
  im=Image.new('RGBA',(w*scale,h*scale));d=ImageDraw.Draw(im)
  for part,points in curves:
   p=pose(points,part,frame);d.line([(round((x*.7+21)*scale),round((y*.72+5)*scale)) for x,y in p],fill='#f2efdf',width=3,joint='curve')
  film.paste(im.resize((w,h),Image.Resampling.LANCZOS),(0,h*(1+phase)))
 if style==0:
  im=Image.new('RGBA',(w*scale,h*scale));d=ImageDraw.Draw(im)
  for part,points in curves:d.line([(round((x*.7+21)*scale),round((y*.72+5)*scale)) for x,y in points],fill='#f2efdf',width=3,joint='curve')
  film.paste(im.resize((w,h),Image.Resampling.LANCZOS),(0,h*61))
 film.quantize(colors=32,method=Image.Quantize.FASTOCTREE).save(r/f'vst/mascot-{style}.png')
# Slanted serif wordmark, small orange crossbar; font license is system DejaVu.
font='/usr/share/fonts/truetype/dejavu/DejaVuSerifCondensed-BoldItalic.ttf'
im=Image.new('RGBA',(600,104));d=ImageDraw.Draw(im);f=ImageFont.truetype(font,70)
d.text((5,-2),'TinyFreaky',font=f,fill='#f2efdf',stroke_width=0)
d.line((16,87,112,87),fill='#f18a3c',width=4)
im=im.resize((300,52),Image.Resampling.LANCZOS);im.save(r/'desktop/resources/wordmark.png');im.save(r/'assets/wordmark.png')
print('540 bounded poses; nine shared compact strips and small native mesh generated')
