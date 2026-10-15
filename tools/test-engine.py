#!/usr/bin/env python3
"""Build C/C++ engine tests using the same source manifest as the Force build."""
import json,os,subprocess,tempfile
from pathlib import Path
r=Path(__file__).resolve().parents[1];cfg=json.loads((r/'vst/vst.json').read_text())['build']
flags=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-DTEST','-I'+str(r/'third_party/eurorack'),'-I'+str(r/'src')]
with tempfile.TemporaryDirectory(prefix='tinyfreaky-test-') as tmp:
 objs=[]
 for i,src in enumerate(cfg['sources']):
  cpp=Path(src).suffix!='.c';out=Path(tmp)/f'{i}.o';subprocess.run(['g++' if cpp else 'cc','-std=c++17' if cpp else '-std=c11',*flags,'-c',str(r/src),'-o',str(out)],check=True);objs.append(str(out))
 for name in ['engine_test','screen_test','factory_test','rhythm_test']:
  obj=Path(tmp)/(name+'.o');binary=Path(tmp)/name
  subprocess.run(['cc','-std=c11',*flags,'-c',str(r/'tests'/(name+'.c')),'-o',str(obj)],check=True)
  subprocess.run(['g++',*flags,*(objs[1:] if name=='rhythm_test' else objs),str(obj),'-pthread','-lm','-o',str(binary)],check=True)
  subprocess.run([str(binary)],env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'),check=True)
