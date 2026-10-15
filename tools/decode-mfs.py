#!/usr/bin/env python3
"""Decode observed MCC .mfs signed-byte vectors to 16-bit/32kHz mono WAV.

This inspects container data; it does not run firmware or implement sample engines.
Rate/channel format is documented in Arturia's MicroFreak V5 manual, section 17.6.
"""
import argparse,hashlib,json,re,wave
from pathlib import Path
LIMIT=8*1024*1024
class Reader:
 def __init__(self,text):self.text=text;self.pos=0
 def token(self):
  match=re.compile(r'\s*(\S+)').match(self.text,self.pos)
  if not match:raise ValueError('Truncated archive')
  self.pos=match.end();return match[1]
 def integer(self):return int(self.token())
 def string(self):
  length=self.integer()
  if length<0 or length>4096:raise ValueError('Invalid string length')
  if self.pos>=len(self.text) or not self.text[self.pos].isspace():raise ValueError('Missing string separator')
  self.pos+=1;value=self.text[self.pos:self.pos+length];self.pos+=length
  if len(value)!=length:raise ValueError('Truncated string')
  return value

def decode(data):
 if len(data)>LIMIT:raise ValueError('Sample exceeds container limit')
 r=Reader(data.decode('ascii'))
 if [r.token(),r.token()]!=['22','serialization::archive']:raise ValueError('Not a Boost text archive')
 archive_version=r.integer()
 if [r.integer(),r.integer()]!=[0,4]:raise ValueError('Unknown MFS object header')
 build=r.string();name=r.string()
 if [r.integer(),r.integer(),r.integer()]!=[1,0,0]:raise ValueError('Unknown MFS metadata layout')
 metadata=r.string();empty=r.integer()
 if empty not in [0,1] or [r.integer(),r.integer()]!=[0,1]:raise ValueError('Unknown MFS vector layout')
 size=r.integer()
 if size<0 or size>24*32000*2 or size%2:raise ValueError('Invalid PCM byte count')
 values=[r.integer() for _ in range(size)]
 if any(v< -128 or v>127 for v in values):raise ValueError('PCM vector contains a non-byte value')
 if r.text[r.pos:].strip():raise ValueError('Trailing unknown records')
 if bool(empty)!=(not bool(size)):raise ValueError('Empty flag/byte count disagree')
 return dict(name=name,build=build,archive_version=archive_version,metadata=metadata,empty=bool(empty),sample_rate=32000,channels=1,sample_width=2,frames=size//2),bytes(v&255 for v in values)

def main():
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('output',type=Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
 files=sorted(a.source.rglob('*.mfs')) if a.source.is_dir() else [a.source];entries=[]
 for i,f in enumerate(files):
  data=f.read_bytes();info,pcm=decode(data);info.update(source=str(f.relative_to(a.source) if a.source.is_dir() else f.name),source_sha256=hashlib.sha256(data).hexdigest(),pcm_sha256=hashlib.sha256(pcm).hexdigest())
  if pcm:
   dest=a.output/(f'{i+1:03d}-'+re.sub(r'[^A-Za-z0-9_-]','_',info['name'])+'.wav')
   if dest.exists():raise ValueError('Refusing to overwrite '+str(dest))
   with wave.open(str(dest),'wb') as w:w.setnchannels(1);w.setsampwidth(2);w.setframerate(32000);w.writeframes(pcm)
   info['wav']=dest.name
  entries.append(info)
 (a.output/'index.json').write_text(json.dumps({'schema':'tinyfreaky.samples/1','entries':entries},indent=2)+'\n');print(f'{len(entries)} slots; {sum(not e["empty"] for e in entries)} samples decoded; empty slots preserved in index')
if __name__=='__main__':main()
