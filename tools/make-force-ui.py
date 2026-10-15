#!/usr/bin/env python3
"""Readable Force pages with a live TinyFreaky OLED, preserving numeric IDs."""
from pathlib import Path
import json,html
ROOT=Path(__file__).resolve().parents[1]
params=json.loads((ROOT/'vst/params.json').read_text());bykey={p['key']:p for p in params}
for key in ['screen_preset','screen_model','screen_detail','screen_hint']:
 if key not in bykey:params.append(dict(key=key,name=key,display='string'))
for i in range(36):
 key='screen_scope_'+str(i) if i<32 else ['screen_env','screen_lfo','screen_state','screen_mascot'][i-32]
 if key not in bykey:params.append(dict(key=key,name=key,min=0,max=3 if i==34 else 16,default=8 if i<32 or i==33 or i==35 else 0,display='int',display_only=True))
(ROOT/'vst/params.json').write_text(json.dumps(params,indent=2)+'\n')
(ROOT/'src/screen_names.inc').write_text('\n'.join(json.dumps(p['name'])+',' for p in params if p.get('display')!='string' and not p.get('display_only'))+'\n')
(ROOT/'src/screen_options.inc').write_text('\n'.join(json.dumps('\t'.join(p.get('options',[])))+',' for p in params if p.get('display')!='string' and not p.get('display_only'))+'\n')
svg='''<svg xmlns="http://www.w3.org/2000/svg" width="1280" height="628"><defs><linearGradient id="chassis" x1="0" y1="0" x2="0.7" y2="1"><stop offset="0" stop-color="#183a30"/><stop offset="0.38" stop-color="#143329"/><stop offset="1" stop-color="#0e251f"/></linearGradient><linearGradient id="panel" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#1c4035"/><stop offset="0.16" stop-color="#193b31"/><stop offset="0.6" stop-color="#17372e"/><stop offset="1" stop-color="#133027"/></linearGradient></defs><rect width="1280" height="628" fill="url(#chassis)"/><text x="26" y="42" fill="#f2efdf" font-family="DejaVu Serif" font-style="italic" font-size="36" font-weight="bold">TinyFreaky</text><text x="1252" y="40" text-anchor="end" fill="#f18a3c" font-family="DejaVu Sans" font-size="24">MagicStino / 0.3.6</text><rect x="24" y="60" width="1232" height="174" rx="12" fill="#070f15" stroke="#507d84" stroke-width="2"/><path d="M48 118H1230 M48 176H1230" stroke="#243c46"/><rect x="24" y="252" width="1232" height="366" rx="12" fill="url(#panel)" stroke="#507260"/></svg>'''
(ROOT/'vst/readable-panel.svg').write_text(svg)
lines=['style=dark','theme_bg=102b25','theme_panel=16382f','theme_ink=f2efdf','theme_ink_dim=b8db91','theme_accent=f18a3c','theme_accent_hi=b8db91','theme_knob_face=f18a3c']

def header(tab,title):
 lines.extend([f'[tab {tab}]','art file=readable-panel.svg',f'text cx=646 cy=119 label="{title}" size=2.4 font="Titillium Web" weight=600 color=b8db91',
 'readout box=0 cx=448 cy=177 w=800 h=50 label="" key=screen_preset tsize=36 tcolor=d9f8ff tpad=0 talign=left',
 'readout box=0 cx=448 cy=234 w=800 h=48 label="" key=screen_model tsize=44 tcolor=f2efdf tpad=0 talign=left',
 'readout box=0 cx=1030 cy=289 w=400 h=48 label="" key=screen_hint tsize=28 tcolor=f18a3c tpad=0 talign=right',
 'readout box=0 cx=448 cy=289 w=800 h=48 label="" key=screen_detail tsize=32 tcolor=d9f8ff tpad=0 talign=left'])
 lines.append('text cx=862 cy=202 label="E L" size=1.6 font="Titillium Web" weight=600 color=b8db91')
 lines.append('text cx=1040 cy=177 label="OUTPUT SCOPE / AUTO" size=2.2 font="Titillium Web" weight=600 color=b8db91')
 for i in range(32):lines.append(f'meter cx={895+i*10} cy=234 w=10 h=56 key=screen_scope_{i} strip=scope-strip.png frames=17')
 lines.extend(['meter cx=1226 cy=177 w=32 h=32 key=screen_state strip=state-strip.png frames=4',
 'meter cx=854 cy=234 w=8 h=56 key=screen_env strip=env-strip.png frames=17',
 'meter cx=870 cy=234 w=8 h=56 key=screen_lfo strip=lfo-strip.png frames=153'])
 if tab!='Matrix':lines.append('meter cx=640 cy=589 w=76 h=166 key=screen_mascot strip=mascot-strip.png frames=17')

def knobs(keys,labels=None,bank=None):
 for i,key in enumerate(keys):
  label=labels[i] if labels else bykey[key]['name'].upper()
  if key in ['seq_reset','all_notes_off']:
   caption='RESTART' if key=='seq_reset' else 'PANIC';name=f'button-{key}.svg'
   (ROOT/'vst'/name).write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="260" height="100"><rect x="2" y="2" width="256" height="96" rx="14" fill="#f18a3c" stroke="#f2efdf" stroke-width="2"/><text x="130" y="64" text-anchor="middle" font-family="DejaVu Sans" font-size="36" font-weight="bold" fill="#102b25">{caption}</text></svg>')
   lines.append(f'button cx={160+(i%4)*320} cy={386+(i//4)*180+35} label="" key={key} img={name} w=260 h=100'+(f' banks="{bank}"' if bank else ''));continue
  color='white' if key in ['osc_type','preset'] else 'black' if key in ['filter_type','cyc_mode','lfo_shape','rhythm_sync','paraphonic','amp_mod','lfo_sync','hold'] else 'orange' if key in ['arp_on','seq_on','arp_mode'] or key.startswith('osc_') else 'green'
  lines.append(f'knob cx={160+(i%4)*320} cy={386+(i//4)*180} r=35 key={key} label="{label}" ns=35 vs=35 bw={252 if i>=4 else 290} img=knob-{color}.svg'+(f' banks="{bank}"' if bank else ''))

pages=[('Oscillator',['osc_type','osc_wave','osc_timbre','osc_shape','preset','master','glide','paraphonic'],['ENGINE','WAVE','TIMBRE','SHAPE','PRESET','MASTER','GLIDE','PARAPHONIC']),('Filter + ENV',['filter_type','cutoff','resonance','filter_amount','attack','decay','sustain','amp_mod'],['MODE','CUTOFF','RESONANCE','ENV AMOUNT','ATTACK','DECAY / REL','SUSTAIN','AMP ENV']),('Cycling ENV',['cyc_mode','cyc_rise','cyc_fall','cyc_hold','cyc_amount','cyc_rise_curve','cyc_fall_curve','tempo'],['MODE','RISE','FALL','HOLD','AMOUNT','RISE CURVE','FALL CURVE','TEMPO']),('LFO + Play',['lfo_shape','lfo_rate_free','lfo_sync','lfo_rate_sync','hold','bend_range','octave','all_notes_off'],['SHAPE','RATE','SYNC','DIVISION','HOLD','BEND RANGE','OCTAVE','PANIC'])]
for title,keys,labels in pages:
 header(title,title.upper());knobs(keys,labels);lines.append(f'qlinks "{title}" = '+','.join(keys))
header('Matrix','MODULATION MATRIX')
for c,label in enumerate(['PITCH','WAVE','TIMBRE','CUTOFF','ASSIGN 1','ASSIGN 2','ASSIGN 3']):
 lines.append(f'text cx={308+c*145} cy=359 label="{label}" size=2.8 font="Titillium Web" weight=600 color=f2efdf banks="CYCLING ENV|ENVELOPE|LFO|PRESSURE|KEY / ARP"')
for r,src in enumerate(['CYCLING ENV','ENVELOPE','LFO','PRESSURE','KEY / ARP']):
 cy=410+r*64
 lines.append(f'text cx=128 cy={cy} label="{src}" size=2.8 font="Titillium Web" weight=600 color=b8db91 banks="CYCLING ENV|ENVELOPE|LFO|PRESSURE|KEY / ARP"')
 for c in range(7):
  lines.append(f'knob lay=side cx={308+c*145} cy={cy} r=14 bw=136 bh=54 key=matrix_{r}_{c} label="" vs=35 img=knob-orange.svg banks="CYCLING ENV|ENVELOPE|LFO|PRESSURE|KEY / ARP"')
 lines.append(f'qlinks "{src}" = '+','.join(f'matrix_{r}_{c}' for c in range(7)))
knobs(['assign1','assign2','assign3'],['ASSIGN 1','ASSIGN 2','ASSIGN 3'],'ASSIGNMENTS');lines.append('qlinks "ASSIGNMENTS" = assign1,assign2,assign3')
header('Rhythm','ARPEGGIATOR / SEQUENCER')
for bank,keys,labels in [
 ('ARPEGGIATOR',['arp_on','arp_mode','arp_rate','swing','arp_octaves','arp_gate','rhythm_sync','tempo'],['ARP ON','MODE','RATE','SWING','OCTAVES','GATE','CLOCK','TEMPO']),
 ('SEQUENCER',['seq_on','seq_rate','seq_length','swing','seq_gate','seq_velocity','seq_transpose','seq_reset'],['PLAY','RATE','LENGTH','SWING','GATE','VELOCITY','TRANSPOSE','RESTART'])]:
 knobs(keys,labels,bank);lines.append(f'qlinks "{bank}" = '+','.join(keys))
for start in range(0,64,8):
 bank=f'STEPS {start+1}-{start+8}';keys=[f'seq_note_{i}' for i in range(start,start+8)]
 knobs(keys,[f'STEP {i+1}' for i in range(start,start+8)],bank);lines.append(f'qlinks "{bank}" = '+','.join(keys))
(ROOT/'vst/layout.conf').write_text('\n'.join(lines)+'\n')
p=ROOT/'vst/vst.json';cfg=json.loads(p.read_text());cfg['defines']['PARAM_TEXT_MAX']=128;cfg['version']=3000;p.write_text(json.dumps(cfg,indent=2)+'\n')

header('MagicStino','MAGICSTINO / TINYFREAKY')
lines.extend(['text cx=640 cy=430 label="github.com/MagicStino/microfreeze" size=3.4 color=f2efdf', 'text cx=640 cy=484 label="Source, updates and desktop downloads" size=2.6 color=b8db91'])
(ROOT/'vst/layout.conf').write_text('\n'.join(lines)+'\n')
