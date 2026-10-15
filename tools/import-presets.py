#!/usr/bin/env python3
"""Independent parser from docs/preset-format-spec.json; no vendor parser code.

Imports legacy named controls only. Never labels a translated patch equivalent.
Original files are not extracted to disk or included in distributions.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]
LIMIT = 8 * 1024 * 1024
ORDER = ['BasicWaves','SuperWave','Wavetable','Harmo','KarplusStr','V.Analog',
         'Waveshaper','Two Op.FM','Formant','Chords','Speech','Modal','Noise',
         'Vocoder','Bass','Sawx','Harm','WaveUser','Sample','Scan grains',
         'Cloud grains','Hit grains']
DIRECT = {'VCO.Param1':'osc_wave','VCO.Param2':'osc_timbre','VCO.Param3':'osc_shape',
          'VCF.Cutoff':'cutoff','VCF.Reso':'resonance','Kbd.Glide':'glide',
          'EG2.Attack':'attack','EG2.DecRel':'decay','EG2.Sustain':'sustain',
          'EG1.RiseLvl':'cyc_rise','EG1.FallLvl':'cyc_fall','EG1.Hold':'cyc_hold',
          'EG1.Amount':'cyc_amount'}
ASSIGN = {(0,0):'Oscillator Type',(0,1):'Wave',(0,3):'Timbre',(0,5):'Shape',
          (1,1):'Cutoff',(1,2):'Resonance',(2,1):'Cycling Rise',
          (2,3):'Cycling Fall',(2,4):'Cycling Hold',(2,6):'Cycling Amount',
          (3,0):'Glide',(5,2):'LFO Rate',(6,1):'Attack',
          (6,2):'Decay / Release',(6,3):'Sustain'}

def unpack(packed):
    result = bytearray()
    if any(x > 127 for x in packed):
        raise ValueError('Packed data contains a byte outside 0..127')
    for start in range(0, len(packed), 8):
        block = packed[start:start+8]
        if len(block) < 2:
            raise ValueError('Dangling MSB mask')
        for bit, low in enumerate(block[1:]):
            result.append(low | (((block[0] >> bit) & 1) << 7))
    return bytes(result)

def payload(data):
    if len(data) > LIMIT:
        raise ValueError('Preset exceeds 8 MiB limit')
    text = data.decode('ascii')
    if not text.startswith('22 serialization::archive '):
        raise ValueError('Unsupported preset container; expected Boost text archive')
    tokens = list(re.finditer(r'\S+', text))
    # Find the count by the observed packed prefix, never by a fixed byte offset.
    for i in range(len(tokens)-6):
        if [t.group() for t in tokens[i+1:i+6]] != ['0','35','86','67','79']:
            continue
        try:
            count = int(tokens[i].group())
        except ValueError:
            continue
        if not 8 <= count <= LIMIT or len(tokens) < i+1+count:
            continue
        try:
            packed = bytes(int(t.group()) for t in tokens[i+1:i+1+count])
        except (ValueError, OverflowError):
            continue
        if any(x > 127 for x in packed):
            continue
        return unpack(packed), text[:tokens[i].start()].strip(), text[tokens[i+count].end():].strip()
    raise ValueError('No validated length-prefixed packed preset payload found')

def records(raw):
    fields, i, group = [], 0, None
    while i < len(raw):
        if raw[i:i+2] == b'@ ':
            return fields, raw[i+2:]
        if raw[i:i+1] == b'#' and i+4 <= len(raw):
            candidate = raw[i+1:i+4]
            if all(32 <= c < 127 for c in candidate):
                group = candidate.decode('ascii'); i += 4; continue
        size = raw[i]-0x40
        if group and 0 < size <= 31 and i+size+5 <= len(raw):
            end = i+1+size
            name = raw[i+1:end]
            if raw[end] == 0x63 and all(32 <= c < 127 for c in name):
                triplet = raw[end+1:end+4]
                fields.append({'group':group,'name':name.decode('ascii'),
                               'offset':i,'resolution':triplet[0],
                               'q15':struct.unpack('<H',triplet[1:])[0],
                               'raw':triplet.hex()})
                i=end+4; continue
        if raw[i] == 0x40:
            group = None
        i += 1
    return fields, b''

def discrete(f):
    return int(math.floor(f['q15']*f['resolution']/32768+0.5))

def translate(data, name, params):
    raw, prefix, suffix = payload(data)
    items, sequence = records(raw)
    if not any(f['group']=='VCO' and f['name']=='Type' for f in items):
        raise ValueError('No oscillator type field; refusing an unidentified patch')
    model_supported = True
    values, consumed, warnings = {}, set(), [
        'Original sound equivalence is unverified; selected oscillator paths use licensed Mutable Instruments cores.',
        'Time/rate/cutoff response curves have not been measured against hardware.'
    ]
    bykey={p['key']:p for p in params}
    def put(key,value,f):
        p=bykey[key]
        lo,hi=(0,len(p['options'])-1) if 'options' in p else (p.get('min',0),p.get('max',1))
        if value < lo-0.001 or value > hi+0.001:
            warnings.append(f'{f["group"]}.{f["name"]}: value outside target range; not mapped')
            return
        values[key]=min(hi,max(lo,value)); consumed.add(f['offset'])
    for f in items:
        field=f['group']+'.'+f['name']; q=f['q15']/32768
        if field in DIRECT:
            put(DIRECT[field],q,f)
        elif field=='VCO.Type':
            idx=max(0,discrete(f)-1)
            if idx>=len(ORDER):
                model_supported = False
                warnings.append('Unknown oscillator selector'); continue
            label=ORDER[idx]
            opts=bykey['osc_type']['options']
            if label not in opts:
                model_supported = False
                warnings.append('Unsupported oscillator '+label); continue
            put('osc_type',opts.index(label),f)
            if f['resolution'] > 18:
                warnings.append('V5 oscillator enumeration is not fixture-verified')
        elif field in ('Gen.Parafon','EG2.Mode','Kbd.Hold','LFO.Sync'):
            put({'Gen.Parafon':'paraphonic','EG2.Mode':'amp_mod',
                 'Kbd.Hold':'hold','LFO.Sync':'lfo_sync'}[field],discrete(f),f)
        elif field in ('VCF.Type','VCF.DType','LFO.Shape','VCO.BendRng'):
            put({'VCF.Type':'filter_type','VCF.DType':'filter_type',
                 'LFO.Shape':'lfo_shape','VCO.BendRng':'bend_range'}[field],discrete(f),f)
        elif field in ('Arp.Range','Arp.Swing','Arp.Dir','Arp.Div'):
            key={'Arp.Range':'arp_octaves','Arp.Swing':'swing','Arp.Dir':'arp_mode','Arp.Div':'arp_rate'}[field]
            value=discrete(f)+(1 if field=='Arp.Range' else 50 if field=='Arp.Swing' else 3 if field=='Arp.Div' else 0)
            put(key,value,f)
        elif field=='Arp.Enable':
            sequence_on=any(x['group']=='Arp' and x['name']=='SeqOn' and discrete(x) for x in items)
            if not sequence_on:put('arp_on',discrete(f),f)
        elif field=='Kbd.Octave':
            put('octave',discrete(f)-3,f)
        elif field=='EG1.Mode':
            # The numeric cycle-mode order is not established by format research.
            warnings.append('Cycling mode selector retained but not mapped (order unverified)')
        elif field in ('EG1.RiseSlp','EG1.FallSlp'):
            put('cyc_rise_curve' if f['name']=='RiseSlp' else 'cyc_fall_curve',q*2-1,f)
        elif f['group'] in ['Co'+str(i) for i in range(1,8)]:
            row={'EG1':0,'EG2':1,'LFO':2,'Xpr':3,'Key':4}.get(f['name'])
            if row is not None:
                signed=struct.unpack('<h',struct.pack('<H',f['q15']))[0]
                put(f'matrix_{row}_{int(f["group"][2:])-1}',signed/32768*100,f)
        elif f['group']=='Mat' and f['name'] in ('Assign1','Assign2','Assign3'):
            group,control=f['q15']>>8,f['q15']&255
            label=ASSIGN.get((group,control))
            if 10<=group<=16 and control<=4:
                label='Matrix '+str(control*7+(group-10)+1)
            if label in bykey[f['name'].lower()]['options']:
                put(f['name'].lower(),bykey[f['name'].lower()]['options'].index(label),f)
            else:
                warnings.append(f'{field}: assignment target {group}:{control} is unsupported')
    unknown=[f for f in items if f['offset'] not in consumed]
    if sequence:
        warnings.append('Binary sequence/modulation-track region preserved, not imported')
    if any(f['group']=='Arp' and f['name']=='Enable' and discrete(f) for f in items):
        warnings.append('Original sequence/modulation tracks and free-rate curve are not yet decoded; arpeggiator controls are mapped where identified')
    model=int(values.get('osc_type',0))
    if model>=16:
        warnings.append('Selected engine needs unavailable wavetable/sample/audio-input support')
    return {'schema':'microfreeze.import/1','name':name,'status':'partial',
            'source_sha256':hashlib.sha256(data).hexdigest(),'parameters':values,
            'warnings':warnings,'records':items,'unmapped_records':unknown,
            'sequence_hex':sequence.hex(),'source_prefix':prefix,'source_suffix':suffix,
            'engine_loadable':model_supported and model<16}

def members(path):
    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as z:
            total=0
            for info in z.infolist():
                if info.is_dir() or Path(info.filename).suffix.lower() not in ('.mbp','.mfp'):
                    continue
                total+=info.file_size
                if info.file_size>LIMIT or total>64*LIMIT:
                    raise ValueError('Archive exceeds import size limit')
                yield Path(info.filename).stem,z.read(info)
    else:
        if path.stat().st_size>LIMIT: raise ValueError('Preset exceeds import size limit')
        yield path.stem,path.read_bytes()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('input',type=Path); ap.add_argument('output',type=Path)
    ap.add_argument('--allow-partial',action='store_true',help='Also emit loadable .mfz files; sound/sequence equivalence is not established')
    a=ap.parse_args(); a.output.mkdir(parents=True,exist_ok=True)
    params=json.loads((ROOT/'vst/params.json').read_text())
    numeric=[p for p in params if p.get('display')!='string']
    for n,(name,data) in enumerate(members(a.input)):
        patch=translate(data,name,params)
        stem=f'{n:03d}-'+re.sub(r'[^A-Za-z0-9_-]','_',name)[:80]
        target=a.output/(stem+'.json')
        if target.exists(): raise ValueError(f'Refusing to overwrite {target}')
        target.write_text(json.dumps(patch,indent=2)+'\n')
        if a.allow_partial and patch['engine_loadable']:
            vals=[]
            for p in numeric:
                default=p.get('default',p.get('min',0))
                if isinstance(default,str): default=p['options'].index(default)
                vals.append(str(patch['parameters'].get(p['key'],default)))
            state='MFZNAME:'+name.replace('\n',' ')+'\nMFZ2;'+';'.join(vals)+';\n'
            (a.output/(stem+'.mfz')).write_text(state)
        print(f'{name}: partial; {len(patch["parameters"])} mapped controls; {len(patch["unmapped_records"])} retained fields')

if __name__=='__main__':
    main()
