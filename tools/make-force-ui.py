#!/usr/bin/env python3
"""Readable Force pages with a live MicroFreak-inspired OLED, preserving numeric IDs."""
from pathlib import Path
import json,html
ROOT=Path(__file__).resolve().parents[1]
params=json.loads((ROOT/'vst/params.json').read_text());bykey={p['key']:p for p in params}
for key in ['screen_preset','screen_model','screen_detail','screen_hint']:
 if key not in bykey:params.append(dict(key=key,name=key,display='string'))
(ROOT/'vst/params.json').write_text(json.dumps(params,indent=2)+'\n')
(ROOT/'src/screen_names.inc').write_text('\n'.join(json.dumps(p['name'])+',' for p in params if p.get('display')!='string')+'\n')
(ROOT/'src/screen_options.inc').write_text('\n'.join(json.dumps('\t'.join(p.get('options',[])))+',' for p in params if p.get('display')!='string')+'\n')
svg='''<svg xmlns="http://www.w3.org/2000/svg" width="1280" height="628"><rect width="1280" height="628" fill="#102b25"/><text x="26" y="42" fill="#f2efdf" font-family="DejaVu Sans" font-size="36" font-weight="bold">tinyfreaky</text><text x="1252" y="40" text-anchor="end" fill="#f18a3c" font-family="DejaVu Sans" font-size="28">0.2.2</text><rect x="24" y="60" width="1232" height="174" rx="12" fill="#070f15" stroke="#507d84" stroke-width="2"/><path d="M48 118H1230 M48 176H1230" stroke="#243c46"/><rect x="24" y="252" width="1232" height="366" rx="12" fill="#16382f" stroke="#507d66"/></svg>'''
(ROOT/'vst/readable-panel.svg').write_text(svg)
lines=['style=dark','theme_bg=102b25','theme_panel=16382f','theme_ink=f2efdf','theme_ink_dim=b8db91','theme_accent=f18a3c','theme_accent_hi=b8db91','theme_knob_face=f18a3c']

def header(tab,title):
 lines.extend([f'[tab {tab}]','art file=readable-panel.svg',f'text cx=646 cy=119 label="{title}" size=1.5 color=b8db91',
 'readout box=0 cx=640 cy=177 w=1176 h=50 label="" key=screen_preset tsize=42 tcolor=d9f8ff tpad=0 talign=left',
 'readout box=0 cx=361 cy=234 w=626 h=48 label="" key=screen_model tsize=32 tcolor=b8e4f1 tpad=0 talign=left',
 'readout box=0 cx=952 cy=234 w=560 h=48 label="" key=screen_hint tsize=28 tcolor=f18a3c tpad=0 talign=right',
 'readout box=0 cx=640 cy=289 w=1176 h=48 label="" key=screen_detail tsize=36 tcolor=d9f8ff tpad=0 talign=left'])

def knobs(keys,labels=None,bank=None):
 for i,key in enumerate(keys):
  label=labels[i] if labels else bykey[key]['name'].upper()
  color='orange' if key.startswith('osc_') else 'green' if key in ['cutoff','resonance','filter_type'] or key.startswith('lfo') else 'orange'
  lines.append(f'knob cx={160+(i%4)*320} cy={386+(i//4)*180} r=35 key={key} label="{label}" ns=35 vs=35 bw=290 img=knob-{color}.svg'+(f' banks="{bank}"' if bank else ''))

pages=[('Oscillator',['osc_type','osc_wave','osc_timbre','osc_shape','preset','master','glide','paraphonic'],['TYPE','WAVE','TIMBRE','SHAPE','PRESET','MASTER','GLIDE','PARAPHONIC']),('Filter + ENV',['filter_type','cutoff','resonance','filter_amount','attack','decay','sustain','amp_mod'],['MODE','CUTOFF','RESONANCE','ENV AMOUNT','ATTACK','DECAY / REL','SUSTAIN','AMP ENV']),('Cycling ENV',['cyc_mode','cyc_rise','cyc_fall','cyc_hold','cyc_amount','cyc_rise_curve','cyc_fall_curve','tempo'],['MODE','RISE','FALL','HOLD','AMOUNT','RISE CURVE','FALL CURVE','TEMPO']),('LFO + Play',['lfo_shape','lfo_rate_free','lfo_sync','lfo_rate_sync','hold','bend_range','octave','all_notes_off'],['SHAPE','RATE','SYNC','DIVISION','HOLD','BEND RANGE','OCTAVE','PANIC'])]
for title,keys,labels in pages:
 header(title,title.upper());knobs(keys,labels);lines.append(f'qlinks "{title}" = '+','.join(keys))
header('Matrix','MODULATION MATRIX')
for r,src in enumerate(['CYCLING ENV','ENVELOPE','LFO','PRESSURE','KEY / ARP']):
 keys=[f'matrix_{r}_{c}' for c in range(7)];knobs(keys,['PITCH','WAVE','TIMBRE','CUTOFF','ASSIGN 1','ASSIGN 2','ASSIGN 3'],src);lines.append(f'qlinks "{src}" = '+','.join(keys))
header('Assign','MATRIX ASSIGNMENTS');knobs(['assign1','assign2','assign3'],['ASSIGN 1','ASSIGN 2','ASSIGN 3']);lines.append('qlinks "Assign" = assign1,assign2,assign3')
(ROOT/'vst/layout.conf').write_text('\n'.join(lines)+'\n')
p=ROOT/'vst/vst.json';cfg=json.loads(p.read_text());cfg['defines']['PARAM_TEXT_MAX']=128;cfg['version']=2020;p.write_text(json.dumps(cfg,indent=2)+'\n')
