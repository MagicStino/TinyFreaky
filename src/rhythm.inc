/* SPDX-License-Identifier: MIT
 * Independent sample-clock arpeggiator and 64-step note sequencer.
 * Gate events are scheduled inside render(), independent of host block size.
 */
static const double rhythm_beats[]={32,16,8,4,2,4.0/3,1,2.0/3,.5,1.0/3,.25,1.0/6,.125,1.0/12};
static void rhythm_off(Synth*s){
 if(s->last_rhythm_note>=0)for(int i=0;i<N;i++)if(s->v[i].stage&&s->v[i].note==s->last_rhythm_note){s->v[i].held=0;s->v[i].stage=4;}
 s->last_rhythm_note=-1;s->gate=0;for(int i=0;i<N;i++)if(s->v[i].held)s->gate=1;
}
static void rhythm_reset(Synth*s){
 rhythm_off(s);for(int i=0;i<N;i++)if(s->v[i].stage){s->v[i].held=0;s->v[i].stage=4;}
 s->gate=0;s->rhythm_countdown=s->rhythm_gate=0;s->rhythm_step=s->seq_step=0;
}
static int rhythm_clock_ready(const Synth*s){return s->p[R_RHYTHM_SYNC]<.5f||(s->transport_seen&&s->transport_playing);}
static int rhythm_display(Synth*s,const char*k,char*b,int n){
 if(!strcmp(k,"swing_display")){snprintf(b,n,"%.0f%%",s->p[R_SWING]);return 1;}
 if(!strcmp(k,"arp_gate_display")||!strcmp(k,"seq_gate_display")){snprintf(b,n,"%.0f%%",s->p[!strcmp(k,"arp_gate_display")?R_ARP_GATE:R_SEQ_GATE]*100);return 1;}
 if(!strncmp(k,"seq_note_",9)){
  char*end;long step=strtol(k+9,&end,10);
  if(step>=0&&step<64&&!strcmp(end,"_display")){
   int pitch=(int)lroundf(s->p[R_SEQ_NOTE_0+step]);static const char*names[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
   if(pitch<0)snprintf(b,n,"REST");else snprintf(b,n,"%s%d",names[pitch%12],pitch/12-1);return 1;
  }
 }
 return 0;
}
static void midi(void*o,const uint8_t*m,int len){
 Synth*s=o;if(len<1)return;int type=m[0]&0xf0;
 if(len>=3&&type==0xb0&&(m[1]==120||m[1]==123)){if(s->p[R_SEQ_ON]>.5f||s->p[R_ARP_ON]>.5f){put(s,"seq_on",0);put(s,"arp_on",0);silence(s);s->rev++;return;}}
 if(len>=3&&(type==0x90||type==0x80)){
  int note=m[1]&127,down=type==0x90&&m[2]>0;
  if(down){int pressed=0;for(int i=0;i<128;i++)pressed+=s->input_held[i]!=0;
   if(!pressed&&s->p[28]>.5f)memset(s->arp_keys,0,sizeof s->arp_keys);
   s->input_held[note]=s->arp_keys[note]=m[2]&127;s->key_order[note]=++s->key_serial;s->seq_root=note-60;
  }else{s->input_held[note]=0;if(s->p[28]<.5f)s->arp_keys[note]=0;}
  if(s->p[R_ARP_ON]>.5f||s->p[R_SEQ_ON]>.5f)return;
 }
 voice_midi(o,m,len);
}
static void rhythm_tick(Synth*s){
 int seq=s->p[R_SEQ_ON]>.5f,arp=s->p[R_ARP_ON]>.5f;
 if((!seq&&!arp)||!rhythm_clock_ready(s)){rhythm_off(s);s->rhythm_countdown=0;return;}
 int held=0;for(int i=0;arp&&i<128;i++)held+=s->arp_keys[i]!=0;
 if(arp&&!held){rhythm_off(s);s->rhythm_countdown=0;s->rhythm_step=0;return;}
 if(s->last_rhythm_note>=0&&--s->rhythm_gate<=0)rhythm_off(s);
 if(s->rhythm_countdown-- >0)return;
 rhythm_off(s);
 int pitch=-1,velocity=100;
 if(seq){int length=(int)s->p[R_SEQ_LENGTH];if(length<1)length=1;if(length>64)length=64;
  s->seq_step=s->rhythm_step%length;int raw=(int)lroundf(s->p[R_SEQ_NOTE_0+s->seq_step]);
  if(raw>=0)pitch=(int)clip(raw+s->p[R_SEQ_TRANSPOSE]+s->seq_root,0,127);
  velocity=(int)clip(s->p[R_SEQ_VELOCITY]*127,1,127);
 }else{
  int notes[512],count=0,octaves=(int)clip(s->p[R_ARP_OCTAVES],1,4),mode=(int)s->p[R_ARP_MODE];
  for(int octave=0;octave<octaves;octave++)for(int note=0;note<128;note++)if(s->arp_keys[note]&&note+octave*12<=127)notes[count++]=note+octave*12;
  if(!count)return;
  if(mode==1){/* Insertion order; octave copies follow the same held-key order. */
   int base[128],used=0;for(int note=0;note<128;note++)if(s->arp_keys[note])base[used++]=note;
   for(int i=1;i<used;i++){int key=base[i],j=i;while(j>0&&s->key_order[base[j-1]]>s->key_order[key]){base[j]=base[j-1];j--;}base[j]=key;}
   count=0;for(int octave=0;octave<octaves;octave++)for(int i=0;i<used;i++)if(base[i]+octave*12<=127)notes[count++]=base[i]+octave*12;
  }
  int selected=s->rhythm_step%count;
  if(mode==2){noise(s);selected=(int)(s->rng%(unsigned)count);}
  if(mode==3){unsigned pattern=(unsigned)(s->rhythm_step%16)+2166136261u;for(int i=0;i<128;i++)if(s->arp_keys[i])pattern=(pattern^(unsigned)i)*16777619u;pattern^=pattern>>13;selected=pattern%(unsigned)count;}
  pitch=notes[selected];velocity=100;
  for(int octave=0;octave<octaves;octave++){int source=pitch-octave*12;if(source>=0&&source<128&&s->arp_keys[source]){velocity=s->arp_keys[source];break;}}
 }
 int rate_id=seq?R_SEQ_RATE:R_ARP_RATE;
 int rate=(int)lroundf(clip(s->p[rate_id]+s->modparam[rate_id],0,13));
 double bpm=s->p[R_RHYTHM_SYNC]>.5f&&s->bpm>0?s->bpm:s->p[20];bpm=fmax(20,fmin(300,bpm));
 double duration=SR*60.0/bpm*rhythm_beats[rate];
 double swing=(clip(s->p[R_SWING],50,75)-50)*.02;
 duration*=s->rhythm_step&1?1-swing:1+swing;
 s->rhythm_countdown+=duration;
 s->rhythm_gate=duration*clip(s->p[seq?R_SEQ_GATE:R_ARP_GATE],.05,1);
 if(pitch>=0){noteon(s,pitch,velocity);s->last_rhythm_note=pitch;}
 s->rhythm_step=(s->rhythm_step+1)%1048576;
}
