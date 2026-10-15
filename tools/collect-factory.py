#!/usr/bin/env python3
"""Preserve all shipped factory bank records; compile supported translations and explicit unavailable slots."""
import argparse,hashlib,importlib.util,json,re,zipfile
from pathlib import Path
from patch_names import patch_name
from preset_tuning import tune
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('preset_import',ROOT/'tools/import-presets.py');imp=importlib.util.module_from_spec(spec);spec.loader.exec_module(imp)
def collect(source,out):
 params=json.loads((ROOT/'vst/params.json').read_text());numeric=[p for p in params if p.get('display')!='string' and not p.get('display_only')];entries={};empty=[];banks={}
 files=sorted(source.rglob('*.mbp'))
 for f in files:
  relative=str(f.relative_to(source));bank=f.relative_to(source).parts[0];banks.setdefault(bank,dict(populated=0,empty=0));data=f.read_bytes()
  try:patch=imp.translate(data,f.stem,params);raw,prefix,_=imp.payload(data)
  except ValueError as e:
   # Only a short archive ending in zero is an empty slot. Other failures abort.
   if len(data)<512 and re.search(rb' 0\s*$',data):empty.append(relative);banks[bank]['empty']+=1;continue
   raise ValueError(f'{relative}: {e}') from e
  match=re.match(r'^22 serialization::archive \d+ \d+ \d+ \d+ \d+ (\d+) ',prefix)
  name=prefix[match.end():match.end()+int(match[1])] if match else f.stem
  digest=hashlib.sha256(raw).hexdigest();banks[bank]['populated']+=1
  if digest in entries:continue
  vals=[]
  for p in numeric:
   d=p.get('default',p.get('min',0));d=p['options'].index(d) if isinstance(d,str) else d
   vals.append(patch['parameters'].get(p['key'],d))
  entries[digest]=dict(name=patch_name(len(entries),vals),sha256=digest,loadable=patch['engine_loadable'],values=tune(vals) if patch['engine_loadable'] else vals,warnings=patch['warnings'],unmapped_count=len(patch['unmapped_records']),has_sequence=bool(patch['sequence_hex']))
 if len(entries)>504:raise ValueError('More than 504 unique factory slots; increase bank capacity before packaging')
 out.mkdir(parents=True,exist_ok=True)
 rows=[]
 for entry in entries.values():
  values=','.join(format(float(v),'.9g')+'f' if '.' in format(float(v),'.9g') or 'e' in format(float(v),'.9g') else str(int(v))+'.0f' for v in entry['values'])
  label=entry['name'] if entry['loadable'] or entry['name'].startswith('Unavailable ') else '[Unavailable] '+entry['name']
  rows.append('{'+json.dumps(label,ensure_ascii=True)+',{'+values+'},'+str(int(entry['loadable']))+'},')
 (ROOT/'src/factory_bank.inc').write_text('/* Generated factory parameter data; source/limitations: assets/factory/index.json. */\n'+'\n'.join(rows)+'\n')
 index=dict(schema='tinyfreaky.factory/2',banks=banks,unique_presets=len(entries),partial_translations=sum(e['loadable'] for e in entries.values()),unsupported=sum(not e['loadable'] for e in entries.values()),empty_slots=len(empty),entries=[{k:v for k,v in e.items() if k!='values'} for e in entries.values()])
 (out/'index.json').write_text(json.dumps(index,indent=2)+'\n');print(json.dumps({k:v for k,v in index.items() if k!='entries'},indent=2))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('--output',type=Path,default=ROOT/'assets/factory');a=p.parse_args();collect(a.source,a.output)
