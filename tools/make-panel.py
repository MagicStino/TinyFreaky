from pathlib import Path
import json,html,math
p=Path(__file__).resolve().parents[1]; controls=[];groups=[]
def group(label,x,y,w,h):groups.append(dict(label=label,x=x,y=y,w=w,h=h))
def knob(key,label,x,y,color='white',r=27):controls.append(dict(key=key,label=label,x=x,y=y,color=color,r=r))
group('MODULATION MATRIX',24,80,430,158)
group('DIGITAL OSCILLATOR',24,252,430,152)
group('ANALOG FILTER',470,252,330,152)
group('CYCLING ENVELOPE',816,252,440,152)
group('LFO',24,420,430,160)
group('ENVELOPE',470,420,440,160)
group('PERFORMANCE',926,420,330,160)
for key,label,x in [('osc_type','TYPE',83),('osc_wave','WAVE',188),('osc_timbre','TIMBRE',293),('osc_shape','SHAPE',398)]:knob(key,label,x,325,'orange')
for key,label,x in [('filter_type','MODE',525),('cutoff','CUTOFF',635),('resonance','RESONANCE',745)]:knob(key,label,x,325,'green')
for key,label,x in [('cyc_rise','RISE',872),('cyc_fall','FALL',980),('cyc_hold','HOLD',1088),('cyc_amount','AMOUNT',1196)]:knob(key,label,x,325)
for key,label,x in [('lfo_shape','SHAPE',83),('lfo_rate_free','RATE',188),('lfo_sync','SYNC',293),('lfo_rate_sync','DIVISION',398)]:knob(key,label,x,493,'green')
for key,label,x in [('attack','ATTACK',526),('decay','DECAY / REL',634),('sustain','SUSTAIN',742),('filter_amount','FILTER AMT',850)]:knob(key,label,x,493)
for key,label,x in [('glide','GLIDE',982),('hold','HOLD',1090),('octave','OCTAVE',1198)]:knob(key,label,x,493)
for key,label,x,y in [('preset','PRESET',1040,148),('master','MASTER',1195,148),('paraphonic','PARAPHONIC',534,158)]:knob(key,label,x,y,'white',31)
panel=dict(width=1280,height=628,groups=groups,controls=controls)
(p/'assets/panel.json').write_text(json.dumps(panel,indent=2)+'\n')
for name,col in {'white':'#f2efdf','green':'#b8db91','orange':'#f18a3c'}.items():
 (p/f'vst/knob-{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="80" height="80"><circle cx="40" cy="40" r="34" fill="{col}"/><path d="M40 14V34" stroke="#102b25" stroke-width="3"/></svg>')
colors=dict(bg='#102b25',panel='#16382f',line='#769b7e',white='#f2efdf',orange='#f18a3c',green='#b8db91',muted='#a2b5a7')
def svg(knobs=False):
 s=['<svg xmlns="http://www.w3.org/2000/svg" width="1280" height="628">',f'<rect width="1280" height="628" fill="{colors["bg"]}"/>']
 def text(x,y,t,size=15,fill='white',anchor='start'):
  s.append(f'<text x="{x}" y="{y}" fill="{colors.get(fill,fill)}" font-family="DejaVu Sans,sans-serif" font-size="{size}" font-weight="600" text-anchor="{anchor}">{html.escape(t)}</text>')
 text(26,46,'tinyfreaky',36);text(282,43,'EXPERIMENTAL HYBRID SYNTH',13,'green');text(1252,42,'MAGICSTINO  /  0.2',13,'orange','end')
 for g in groups:
  x,y,w,h=[g[k] for k in ['x','y','w','h']];s.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="7" fill="{colors["panel"]}" stroke="{colors["line"]}" stroke-opacity=".45"/>');text(x+14,y+25,g['label'],14,'green')
 s.append('<rect x="612" y="98" width="326" height="118" rx="6" fill="#091910" stroke="#b8db91"/>');text(631,125,'TINYFREAKY',14,'green');text(631,159,'OSCILLATOR / PRESET',20);text(631,190,'Prototype engine · MIDI input',13,'orange')
 for r,l in enumerate(['CYC','ENV','LFO','PRESS','KEY']):
  text(37,131+r*20,l,10,'muted')
  for c in range(7):s.append(f'<circle cx="{113+c*47}" cy="{127+r*20}" r="3" fill="{colors["orange"] if (r+c)%5==0 else colors["green"]}" opacity=".8"/>')
 for c,l in enumerate(['PITCH','WAVE','TIMBRE','CUTOFF','ASG 1','ASG 2','ASG 3']):text(113+c*47,113,l,8,'white','middle')
 text(26,610,'PANEL  /  MATRIX  /  ADVANCED',12,'green');text(1252,610,'RESEARCH BUILD · SOUND EQUIVALENCE UNVERIFIED',11,'muted','end')
 if knobs:
  for k in controls:
   x,y,r=[k[v] for v in ['x','y','r']];col=colors[k['color']]
   s.append(f'<circle cx="{x}" cy="{y}" r="{r+5}" fill="none" stroke="{col}" stroke-opacity=".5"/><circle cx="{x}" cy="{y}" r="{r}" fill="{col}"/><line x1="{x}" y1="{y}" x2="{x+8}" y2="{y-r+6}" stroke="#102b25" stroke-width="3"/>');text(x,y+r+25,k['label'],11,'white','middle')
 s.append('</svg>');return '\n'.join(s)
(p/'vst/panel.svg').write_text(svg());(p/'assets/tinyfreaky-panel.svg').write_text(svg(True))
lines=['style=dark','theme_bg=102b25','theme_panel=16382f','theme_ink=f2efdf','theme_ink_dim=a2b5a7','theme_accent=f18a3c','theme_accent_hi=b8db91','theme_knob_face=f2efdf','[tab Panel]','art file=panel.svg']
for k in controls:lines.append(f'knob cx={k["x"]} cy={k["y"]} r={k["r"]} key={k["key"]} label="{k["label"]}" ns=11 vs=14 bw=100')
lines+=['qlinks "Oscillator" = osc_type,osc_wave,osc_timbre,osc_shape,cutoff,resonance,filter_type,filter_amount,attack,decay,sustain,master,cyc_rise,cyc_fall,cyc_hold,cyc_amount','qlinks "LFO and performance" = lfo_shape,lfo_rate_free,lfo_sync,lfo_rate_sync,glide,hold,octave,paraphonic,preset,master']
lines+=['[tab Matrix]','text cx=640 cy=35 label="TINYFREAKY / MODULATION MATRIX" size=1.3 color=b8db91']
for r,src in enumerate(['CYC ENV','ENV','LFO','PRESSURE','KEY']):
 for c,dst in enumerate(['PITCH','WAVE','TIMBRE','CUTOFF','ASSIGN 1','ASSIGN 2','ASSIGN 3']):lines.append(f'knob cx={100+c*180} cy={112+r*102} r=22 key=matrix_{r}_{c} label="{src} > {dst}" ns=11 vs=14 bw=154')
 lines.append(f'qlinks "{src}" = '+','.join(f'matrix_{r}_{c}' for c in range(7)))
lines+=['[tab Advanced]','text cx=640 cy=40 label="TINYFREAKY / SHIFTED FUNCTIONS" size=1.4 color=b8db91']
extra=['cyc_mode','cyc_rise_curve','cyc_fall_curve','amp_mod','tempo','bend_range','assign1','assign2','assign3','all_notes_off']
for i,key in enumerate(extra):lines.append(f'knob cx={140+(i%5)*245} cy={190+(i//5)*200} r=34 key={key} label="{key.replace("_"," ").upper()}" ns=14 vs=20 bw=200')
import re
for i,line in enumerate(lines):
 lines[i]=re.sub(r'cy=(\d+)',lambda m:'cy='+str(int(m[1])+86),line)
 if line.startswith('knob'):
  key=line.split('key=')[1].split()[0];color=next((k['color'] for k in controls if k['key']==key),'white');lines[i]+=' img=knob-'+color+'.svg'
(p/'vst/layout.conf').write_text('\n'.join(lines)+'\n')
cfg=json.loads((p/'vst/vst.json').read_text());cfg['art']='html';(p/'vst/vst.json').write_text(json.dumps(cfg,indent=2)+'\n')
# Shared coordinates and keys are consumed directly by the desktop editor.
(p/'desktop/panel.inc').write_text('\n'.join('{"%(key)s","%(label)s",%(x)s,%(y)s,%(r)s,"%(color)s"},'%k for k in controls)+'\n')
(p/'desktop/groups.inc').write_text('\n'.join('{"%(label)s",%(x)s,%(y)s,%(w)s,%(h)s},'%g for g in groups)+'\n')

# Force uses the readable layout; the compact design above remains a desktop reference.
import runpy
runpy.run_path(str(p/"tools/make-force-ui.py"),run_name="__main__")
