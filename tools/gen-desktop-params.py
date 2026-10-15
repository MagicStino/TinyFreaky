#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import json
from pathlib import Path
project=Path(__file__).resolve().parents[1]
ps=json.loads((project/'vst/params.json').read_text()); out=[]
for p in ps:
 if p.get('display')=='string':continue
 opts=p.get('options',[]);lo=p.get('min',0);hi=len(opts)-1 if opts else p.get('max',1);d=p.get('default',lo)
 if isinstance(d,str):d=opts.index(d)
 steps=len(opts)-1 if opts else (int(hi-lo) if p.get('display')=='int' or p.get('momentary') else 0)
 out.append('{'+','.join([json.dumps(p['key']),json.dumps(p['name']),str(lo),str(hi),str(d),str(steps)])+'},')
(project/'desktop/parameters.inc').write_text('\n'.join(out)+'\n')
(project/'desktop/options.inc').write_text('\n'.join(json.dumps('\t'.join(p.get('options',[])))+',' for p in ps if p.get('display')!='string')+'\n')
print(len(out))
