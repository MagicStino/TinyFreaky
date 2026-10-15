/* SPDX-License-Identifier: MIT
 * Independent instrument. DSP remains approximate. Factory parameter data is included with provenance.
 * Model names describe the public control inventory, not sonic equivalence.
 */
#include "engine.h"
#include "plaits_bridge.h"
#include "rhythm_params.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <dirent.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SR 44100.f
#define PI 3.14159265358979323846f
#define N 4
static const struct Param {const char *key; float def,lo,hi;} defs[]={
#include "parameters.inc"
};
#define NP ((int)(sizeof(defs)/sizeof(defs[0])))
typedef struct {int note,held,stage; unsigned age; float phase,sub,detune[2],env,vel,pressure,freq,last,pluck[2048];int pos;} Voice;
typedef struct {char name[128];float values[NP];int playable;} Patch;
static const Patch factory_bank[]={
#include "factory_bank.inc"
};
#define FACTORY_COUNT ((int)(sizeof factory_bank/sizeof *factory_bank))
typedef struct {Patch *bank;void*plaits[N];int count,active_model;char status[192];int focus;float p[NP],modparam[NP];Voice v[N];unsigned rng,age,rev;float bend,pressure,lphase,rand0,rand1,cyc;int cstage,gate;float env,low,band,dc,dc_input,holdtime,matrixmod[35];double bpm;unsigned char input_held[128],arp_keys[128];unsigned key_order[128],key_serial;double rhythm_countdown,rhythm_gate;int rhythm_step,seq_step,last_rhythm_note,seq_root,transport_seen,transport_playing;double transport_ppq;} Synth;
static float clip(float x,float a,float b){return x<a?a:x>b?b:x;}
static int idx(const char*k){for(int i=0;i<NP;i++)if(!strcmp(k,defs[i].key))return i;return -1;}
static void put(Synth*s,const char*k,float x){int i=idx(k);if(i>=0&&isfinite(x)){s->p[i]=clip(x,defs[i].lo,defs[i].hi);
 if(i==3)s->active_model=(int)s->p[3];
 if(i==13)s->p[44]=(s->p[13]*2-1)*100;
 if(i==44)s->p[13]=(s->p[44]+100)*.005f;
 }}
static float noise(Synth*s){s->rng^=s->rng<<13;s->rng^=s->rng>>17;s->rng^=s->rng<<5;return (s->rng>>8)*(1.f/8388608.f)-1;}
static float seconds(float x){return .002f*powf(10000.f,clip(x,0,1));}
static float sine(float p){return sinf(2*PI*p);}
static float saw(float p,float dt){float y=2*p-1;if(p<dt){float t=p/dt;y-=2*t-t*t-1;}else if(p>1-dt){float t=(p-1)/dt;y-=t*t+2*t+1;}return y;}
static float cutoff_hz(float x){x=clip(x,0,1);return 30.f+19970.f*x*x*x;}
static float pulse(float p,float dt,float width){float edge=p-width;if(edge<0)edge+=1;return saw(edge,dt)-saw(p,dt)+(2*width-1);}
static float superwave(float p,float dt,float wave){float x=wave*3;int i=(int)x;float a=i==0?saw(p,dt):i==1?pulse(p,dt,.5f):i==2?2.f/PI*asinf(sine(p)):sine(p);float b=i==0?pulse(p,dt,.5f):i==1?2.f/PI*asinf(sine(p)):sine(p);return a+(b-a)*(x-i);}
static float output_limit(float x){float a=fabsf(x);return a<=.8f?x:copysignf(.8f+.2f*tanhf((a-.8f)*5),x);}
static void silence(Synth*s){s->p[13]=(s->p[44]+100)*.005f;memset(s->v,0,sizeof(s->v));memset(s->modparam,0,sizeof(s->modparam));memset(s->matrixmod,0,sizeof(s->matrixmod));s->gate=0;s->low=s->band=s->dc=s->dc_input=0;s->active_model=(int)s->p[3];memset(s->input_held,0,sizeof s->input_held);memset(s->arp_keys,0,sizeof s->arp_keys);s->rhythm_countdown=s->rhythm_gate=0;s->rhythm_step=s->seq_step=s->seq_root=0;s->last_rhythm_note=-1;for(int i=0;i<N;i++)tf_plaits_reset(s->plaits[i]);}
static void preset(Synth*s,int n){for(int i=0;i<NP;i++)s->p[i]=defs[i].def;put(s,"preset",n);if(n){int types[]={0,1,7,4,9,11,13,8};put(s,"osc_type",types[n%8]);put(s,"osc_wave",.12f*n);put(s,"osc_timbre",.2f+.08f*n);put(s,"cutoff",.4f+.06f*n);put(s,"decay",.2f+.06f*n);if(n==3||n==5)put(s,"sustain",0);put(s,"matrix_2_3",12);}} 
static void scan_bank(Synth*s,const char*dir);
static void *create(const char*dir){Synth*s=calloc(1,sizeof(*s));if(s){for(int i=0;i<N;i++)s->plaits[i]=tf_plaits_create();s->rng=0x6d2b79f5;s->bpm=0;s->last_rhythm_note=-1;s->count=8;s->focus=4;s->active_model=-1;preset(s,0);s->bank=calloc(504,sizeof(Patch));if(s->bank){memcpy(s->bank,factory_bank,sizeof factory_bank);s->count+=FACTORY_COUNT;}scan_bank(s,dir);snprintf(s->status,sizeof(s->status),"Independent prototype: original preset sound unverified");}return s;}
static void destroy(void*p){Synth*s=p;for(int i=0;i<N;i++)tf_plaits_destroy(s->plaits[i]);free(s->bank);free(s);}
static void noteon(Synth*s,int note,int vel){int j=0,n=s->p[2]>=.5?4:1;unsigned oldest=~0u;for(int i=0;i<n;i++){if(!s->v[i].stage){j=i;oldest=0;break;}if(s->v[i].age<oldest){oldest=s->v[i].age;j=i;}}Voice*v=s->v+j;tf_plaits_reset(s->plaits[j]);float prev=v->freq;memset(v,0,sizeof(*v));v->note=note;v->held=1;v->stage=1;v->age=++s->age;v->vel=vel/127.f;v->freq=prev>0?prev:440*powf(2,(note-69)/12.f);for(int i=0;i<2048;i++)v->pluck[i]=noise(s)*.6f;s->gate=1;if((int)s->p[21]!=1){s->cyc=0;s->cstage=0;}}
static void voice_midi(void*o,const uint8_t*m,int len){Synth*s=o;if(!m||len<1)return;int t=m[0]&240;if((t==144||t==128)&&len>=3){if(t==144&&m[2])noteon(s,m[1],m[2]);else for(int i=0;i<N;i++)if(s->v[i].note==m[1]){s->v[i].held=0;if(s->p[28]<.5)s->v[i].stage=4;}}else if(t==224&&len>=3)s->bend=((m[1]+128*m[2])-8192)/8192.f;else if(t==208&&len>=2)s->pressure=m[1]/127.f;else if(t==160&&len>=3){for(int i=0;i<N;i++)if(s->v[i].note==m[1])s->v[i].pressure=m[2]/127.f;}else if(t==176&&len>=3){int c=m[1];if(c==120||c==123){silence(s);return;}const int nums[]={5,9,10,12,13,23,24,26,28,29,64,83,93,94,102,103,105,106};const char*keys[]={"glide","osc_type","osc_wave","osc_timbre","osc_shape","cutoff","cyc_amount","filter_amount","cyc_hold","sustain","hold","resonance","lfo_rate_free","lfo_rate_sync","cyc_rise","cyc_fall","attack","decay"};for(int i=0;i<18;i++)if(c==nums[i]){int j=idx(keys[i]);float x=m[2]/127.f;if(c!=9)put(s,keys[i],defs[j].lo+x*(defs[j].hi-defs[j].lo));if(c==64&&m[2]<64)for(int v=0;v<N;v++)if(!s->v[v].held)s->v[v].stage=4;}}s->gate=0;for(int i=0;i<N;i++)if(s->v[i].held|| (s->p[28]>=.5&&s->v[i].stage))s->gate=1;}
#include "rhythm.inc"
static void set_param(void*o,const char*k,const char*v){Synth*s=o;s->rev++;if(strcmp(k,"state")&&idx(k)>=0)s->focus=idx(k);if(!strcmp(k,"state")){if(strncmp(v,"MFZ1;",5)&&strncmp(v,"MFZ2;",5))return;int count=v[3]=='1'?70:NP;float tmp[NP];for(int i=0;i<NP;i++)tmp[i]=defs[i].def;const char*p=v+5;for(int i=0;i<count;i++){char*e;tmp[i]=strtof(p,&e);if(e==p||!isfinite(tmp[i])||*e!=';')return;p=e+1;}if(*p||tmp[idx("osc_type")]>=16)return;for(int i=0;i<NP;i++)s->p[i]=clip(tmp[i],defs[i].lo,defs[i].hi);silence(s);return;}float x=strtof(v,NULL);if(!strcmp(k,"preset")){int n=(int)roundf(clip(x,0,511));if(n>=s->count){put(s,"preset",n);snprintf(s->status,sizeof(s->status),"Preset slot unavailable; previous sound retained");return;}if(n<8)preset(s,n);else{memcpy(s->p,s->bank[n-8].values,sizeof(s->p));put(s,"preset",n);if(!s->bank[n-8].playable){put(s,"master",0);snprintf(s->status,sizeof(s->status),"Unavailable engine: preset retained, audio muted");}else snprintf(s->status,sizeof(s->status),"Partial factory translation; original sound and sequence unverified");}silence(s);}else if(!strcmp(k,"all_notes_off")){if(x>.5){put(s,"seq_on",0);silence(s);}}else if(!strcmp(k,"seq_reset")){if(x>.5)rhythm_reset(s);}else if(!strcmp(k,"osc_type")&&x>=16){snprintf(s->status,sizeof(s->status),"Unavailable: user table/sample/grain/vocoder engine needs external assets/input");}else if(!strncmp(k,"assign",6)&&((int)x==2)){put(s,k,x);snprintf(s->status,sizeof(s->status),"Inactive assignment: Sample engine unavailable");}else {put(s,k,x);if(!strcmp(k,"arp_on")||!strcmp(k,"seq_on")){if(x>.5)put(s,!strcmp(k,"arp_on")?"seq_on":"arp_on",0);rhythm_reset(s);}if(!strcmp(k,"osc_type"))snprintf(s->status,sizeof(s->status),"%s",(int)x==10?"Speech: original open-source word-bank engine":"Independent instrument: original sound equivalence unverified");if(!strcmp(k,"hold")&&x<.5){memcpy(s->arp_keys,s->input_held,sizeof s->arp_keys);for(int i=0;i<N;i++)if(!s->v[i].held)s->v[i].stage=4;}}}
static const char* labels[22][3]={
{"Morph","Sym","Sub"},
{"Wave","Detune","Volume"},
{"Table","Position","Chorus"},
{"Content","Sculpting","Chorus"},
{"Bow","Position","Decay"},
{"Detune","Shape","Wave"},
{"Wave","Amount","Asym"},
{"Ratio","Amount","Feedback"},
{"Interval","Formant","Shape"},
{"Chord type","Inv/Transp","Waveform"},
{"Category","Formant","Word"},
{"Inharm","Brightness / dust","Decay"},
{"Noise source","Rate reduction","Wave mix"},
{"Saturate","Fold","Noise"},
{"Saw Mod","Noise","Chorus"},
{"Spread","Rectification","Noise"},
{"Table","Position","Bitdepth"},
{"Start","Length","Loop"},
{"Scan","Density","Chaos"},
{"Start","Density","Chaos"},
{"Start","Density","Shape"},
{"Wave","Frequency range","Bandwidth"},
};
static const char* screen_names[]={
#include "screen_names.inc"
};
static const char* screen_options[]={
#include "screen_options.inc"
};
static const char* model_names[]={"Basic Waves","SuperWave","Wavetable","Harmonic","Karplus Strong","Virtual Analog","Waveshaper","Two Operator FM","Formant","Chords","Speech","Modal","Noise","Bass","SawX","Harm"};
static void screen_value(Synth*s,int i,char*b,int n){char key[96];snprintf(key,sizeof key,"%s_display",defs[i].key);if(rhythm_display(s,key,b,n))return;
 const char*o=screen_options[i];if(*o){int k=(int)lroundf(s->p[i]-defs[i].lo);while(k-->0&&strchr(o,'\t'))o=strchr(o,'\t')+1;int len=(int)strcspn(o,"\t");snprintf(b,n,"%.*s",len,o);}
 else if(i==8)snprintf(b,n,"%.0f Hz",cutoff_hz(s->p[i]));
 else if(defs[i].lo==0&&defs[i].hi==1)snprintf(b,n,"%.0f%%",s->p[i]*100);
 else if(i==17)snprintf(b,n,"%.2f Hz",s->p[i]);
 else if(i==20)snprintf(b,n,"%.0f BPM",s->p[i]);
 else snprintf(b,n,"%.0f",s->p[i]);
}
static int screen_read(Synth*s,const char*k,char*b,int n){
 int slot=(int)s->p[0],unavailable=slot>=8&&slot<s->count&&!s->bank[slot-8].playable;
 if(!strcmp(k,"screen_preset")){const char*name=slot>=8&&slot<s->count?s->bank[slot-8].name:NULL;static const char*natives[]={"Init Clean","Wide Air","FM Glass","Pluck Wire","Open Chord","Modal Dust","Fold Bass","Vowel Reed"};if(slot<8)name=natives[slot];if(!name)name="Empty slot";if(!strncmp(name,"~ ",2))name+=2;if(!strncmp(name,"[Unavailable] ",14))name+=14;return snprintf(b,n,"%03d  %s",slot+1,name);}
 if(!strcmp(k,"screen_model")){int m=(int)clip(s->active_model<0?s->p[3]:s->active_model,0,15);return unavailable?snprintf(b,n,"ENGINE UNAVAILABLE"):snprintf(b,n,"ENGINE %02d  %s",m+1,model_names[m]);}
 if(!strcmp(k,"screen_hint")){if(s->p[R_SEQ_ON]>.5f)return snprintf(b,n,"SEQ %02d/%02d  /  %s",s->seq_step+1,(int)s->p[R_SEQ_LENGTH],rhythm_clock_ready(s)?"PLAY":"HOST WAIT");if(s->p[R_ARP_ON]>.5f)return snprintf(b,n,"ARP  /  %s",rhythm_clock_ready(s)?"PLAY":"HOST WAIT");if(unavailable)return snprintf(b,n,"UNAVAILABLE / MUTED");int routes=0,voices=0;for(int i=34;i<69;i++)if(fabsf(s->p[i])>.01f)routes++;for(int i=0;i<N;i++)if(s->v[i].stage)voices++;return snprintf(b,n,"%d ROUTES  /  %s",routes,voices==0?"READY":voices==1?"1 VOICE":voices==2?"2 VOICES":voices==3?"3 VOICES":"4 VOICES");}
 if(!strcmp(k,"screen_detail")){int i=s->focus;char value[128];if(i<=0||i>=NP)i=4;screen_value(s,i,value,sizeof value);return snprintf(b,n,"%s   %s",screen_names[i],value);}
 return 0;
}
static int get_param(void*o,const char*k,char*b,int n){Synth*s=o;if(n<=0)return 0;if(!strcmp(k,"cutoff_display"))return snprintf(b,n,"%.0f Hz",cutoff_hz(s->p[8]));if(!strcmp(k,"display_rev"))return snprintf(b,n,"%u",s->rev);if(rhythm_display(s,k,b,n))return (int)strlen(b);if(!strncmp(k,"screen_",7))return screen_read(s,k,b,n);size_t kl=strlen(k);if(kl>8&&!strcmp(k+kl-8,"_display")&&strcmp(k,"filter_amount_display")){char key[64];if(kl-8<sizeof key){memcpy(key,k,kl-8);key[kl-8]=0;int d=idx(key);if(d>=0&&defs[d].lo==0&&defs[d].hi==1)return snprintf(b,n,"%.0f%%",s->p[d]*100);}}if(!strcmp(k,"filter_amount_display"))return snprintf(b,n,"%+.0f",s->p[44]);if(!strcmp(k,"bank_count"))return snprintf(b,n,"%d",s->count);if(!strcmp(k,"status"))return snprintf(b,n,"%s",s->status);if(!strcmp(k,"state")){int z=snprintf(b,n,"MFZ2;");for(int i=0;i<NP&&z<n;i++)z+=snprintf(b+z,n-z,"%.9g;",s->p[i]);return z<n?z:0;}if(!strncmp(k,"preset:",7)){int slot=atoi(k+7);if(slot<0||slot>=s->count)return snprintf(b,n,"Unavailable slot %d",slot);if(slot>=8&&slot<s->count)return snprintf(b,n,"%s",s->bank[slot-8].name+(!strncmp(s->bank[slot-8].name,"~ ",2)?2:0));static const char*names[]={"Init Clean","Wide Air","FM Glass","Pluck Wire","Open Chord","Modal Dust","Fold Bass","Vowel Reed"};return snprintf(b,n,"%s",names[(int)clip((float)atoi(k+7),0,7)]);}if(!strcmp(k,"osc_wave_name")||!strcmp(k,"osc_timbre_name")||!strcmp(k,"osc_shape_name")){int column=!strcmp(k,"osc_wave_name")?0:!strcmp(k,"osc_timbre_name")?1:2;return snprintf(b,n,"%s",labels[(int)s->p[3]][column]);}int i=idx(k);return i<0?0:snprintf(b,n,"%.9g",s->p[i]);}
static int pathcmp(const void*a,const void*b){return strcmp(*(const char*const*)a,*(const char*const*)b);}
static void scan_bank(Synth*s,const char*dir){
 if(!dir){return;}char*files[504];int count=0;
#ifdef _WIN32
 char pattern[4096];snprintf(pattern,sizeof(pattern),"%s/*",dir);
 WIN32_FIND_DATAA entry;HANDLE handle=FindFirstFileA(pattern,&entry);
 if(handle==INVALID_HANDLE_VALUE)return;
 do {if(entry.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;
  const char*name=entry.cFileName;size_t n=strlen(name);
  if(n<5||strcmp(name+n-4,".mfz"))continue;
  if(count<504){files[count]=malloc(n+1);if(files[count]){memcpy(files[count],name,n+1);count++;}}
 } while(FindNextFileA(handle,&entry));
 FindClose(handle);
#else
 DIR*d=opendir(dir);if(!d)return;struct dirent*e;
 while((e=readdir(d))&&count<504){size_t n=strlen(e->d_name);if(n<5||strcmp(e->d_name+n-4,".mfz"))continue;files[count]=malloc(n+1);if(files[count]){memcpy(files[count],e->d_name,n+1);count++;}}
 closedir(d);
#endif
 qsort(files,count,sizeof(files[0]),pathcmp);
 for(int i=0;i<count;i++){char path[4096];snprintf(path,sizeof(path),"%s/%s",dir,files[i]);FILE*f=fopen(path,"r");if(f&&s->bank&&s->count<512){char line[16384],name[128];snprintf(name,sizeof(name),"%.127s",files[i]);if(fgets(line,sizeof(line),f)){if(!strncmp(line,"MFZNAME:",8)){line[strcspn(line,"\r\n")]=0;snprintf(name,sizeof(name),"%.127s",line+8);if(!fgets(line,sizeof(line),f))line[0]=0;}line[strcspn(line,"\r\n")]=0;float saved[NP];memcpy(saved,s->p,sizeof(saved));s->p[0]=-1;set_param(s,"state",line);if(s->p[0]>=0){Patch*p=s->bank+(s->count-8);memcpy(p->values,s->p,sizeof(p->values));snprintf(p->name,sizeof(p->name),"%s",name);p->playable=1;s->count++;}memcpy(s->p,saved,sizeof(saved));} }if(f)fclose(f);free(files[i]);}
}
static float osc(Synth*s,Voice*v,int model,float hz,float w,float t,float h){if(model>=2&&model<=12)return tf_plaits_sample(s->plaits[v-s->v],model,hz,w,t,h,v->held,v->vel);float dt=clip(hz/SR,.00001,.4),p=v->phase,a=saw(p,dt),b=sine(p),y=0;v->phase+=dt;v->phase-=floorf(v->phase);v->sub+=dt*.5;v->sub-=floorf(v->sub);switch(model){
case 0:y=(1-w)*a+w*pulse(p,dt,.05f+.9f*t);y=y*(1-h)+h*sine(v->sub);break;
case 1:{float spread=t*t*35.f,ratio=powf(2,spread/1200.f),d0=clip(dt/ratio,.00001f,.4f),d1=clip(dt*ratio,.00001f,.4f);v->detune[0]=fmodf(v->detune[0]+d0,1.f);v->detune[1]=fmodf(v->detune[1]+d1,1.f);y=(superwave(p,dt,w)+h*(superwave(v->detune[0],d0,w)+superwave(v->detune[1],d1,w)))/(1+2*h);break;}
case 2:case 16:y=sine(p+t*sine(p*(1+floorf(w*7)))*.3f);y=(y+h*sine(v->sub*2.01f))/(1+h);if(model==16){float q=powf(2,3+12*(1-h));y=roundf(y*q)/q;}break;
case 3:for(int j=1;j<=12;j++)y+=sine(p*j)*expf(-j*(.1f+t))*(.5f+.5f*cosf(j*w*PI));y*=.4f;break;
case 4:{int size=(int)clip(SR/hz,2,2047),i=v->pos%size;float z=v->pluck[i];v->pluck[i]=(z+v->pluck[(i+1)%size])*(.45f+.0495f*h)+noise(s)*w*.003f;v->pos=(i+1)%size;y=z*(.5f+.5f*t);break;}
case 5:y=(1-h)*a+h*(p<.1f+.8f*t?1:-1);y=(y+saw(fmodf(p+.02f*w,1),dt))*.5f;break;
case 6:y=sinf((a*(1-w)+b*w)*(1+12*t)+h*3);break;
case 7:y=sine(p+(1+12*t)*sine(p*(1+floorf(w*15))+v->last*h*.25f)*.16f);v->last=y;break;
case 8:case 10:y=b*.35f+sine(p*(2+floorf(w*5)))*(.2f+.3f*t)+sine(p*(5+floorf(t*12)))*(.15f+.3f*h);break;
case 9:{static const int chords[4][3]={{0,4,7},{0,3,7},{0,5,7},{0,7,12}};int c=(int)(w*3.99f);for(int j=0;j<3;j++)y+=sine(p*powf(2,(chords[c][j]+12*floorf(t*3))/12.f));y=y/3*(1-h)+a*h;break;}
case 11:for(int j=1;j<=6;j++)y+=sine(p*(j+w*j*j*.12f))*expf(-j*(1-t))*.25f; y*=.5f+.5f*h;break;
case 12:{int rate=1+(int)(t*128);if(++v->pos>=rate){v->pos=0;v->last=noise(s);}y=v->last*(1-h)+a*h*(.2f+.8f*w);break;}
case 13:y=tanhf((a*(1-w)+b*w)*(1+8*w));y=sinf(y*(1+5*t))*(1-h)+noise(s)*h*.3f;break;
case 14:y=saw(fmodf(p+w*.1f*sine(p*3)+1,1),dt)*(1-t)+noise(s)*t*.4f;y=(y+h*sine(v->sub))/(1+h);break;
case 15:y=(sine(p)+sine(p*(2+w*4)))*.5f;y=(1-t)*y+t*(fabsf(y)*2-1);y=y*(1-h)+noise(s)*h*.25f;break;
default:y=b;break;}return clip(y,-2,2);}
static void render(void*o,int16_t*out,int frames){Synth*s=o;static const int targetids[]={7,3,-1,4,5,6,8,9,10,11,12,13,17,R_ARP_RATE,22,23,24,25};for(int f=0;f<frames;f++){rhythm_tick(s);
float ep[NP];for(int z=0;z<NP;z++)ep[z]=clip(s->p[z]+s->modparam[z],defs[z].lo,defs[z].hi);memset(s->modparam,0,sizeof(s->modparam));float rate=ep[17];if(ep[18]>.5)rate=(s->bpm>0?s->bpm:ep[20])/60*powf(2,ep[19]-4);s->lphase+=rate/SR;if(s->lphase>=1){s->lphase-=floorf(s->lphase);s->rand0=s->rand1;s->rand1=noise(s);}float lp=s->lphase,lfo;switch((int)ep[16]){case 1:lfo=1-4*fabsf(lp-.5f);break;case 2:lfo=2*lp-1;break;case 3:lfo=lp<.5?1:-1;break;case 4:lfo=s->rand1;break;case 5:lfo=s->rand0+(s->rand1-s->rand0)*lp;break;default:lfo=sine(lp);}
int cm=(int)ep[21];if(cm==1||s->gate||s->cyc>0){float step=1/(SR*seconds(ep[s->cstage==0?22:23]));if(s->cstage==0){s->cyc+=step;if(s->cyc>=1){s->cyc=1;s->cstage=1;s->holdtime=seconds(ep[24])*SR;}}else if(s->holdtime>0&&cm!=0){s->holdtime--;}else if(!(cm==0&&s->gate)){s->cyc-=step;if(s->cyc<=0){s->cyc=0;if(cm!=0)s->cstage=0;}}}
float key=0,pr=s->pressure;int active=0;for(int i=0;i<N;i++)if(s->v[i].stage){key+=(s->v[i].note-60)/24.f;pr=fmaxf(pr,s->v[i].pressure);active++;}if(active)key/=active;
float sources[5]={powf(s->cyc,powf(4,ep[s->cstage==0?26:27]))*ep[25],s->env,lfo,pr,key},mods[7]={0};for(int r=0;r<5;r++)for(int d=0;d<7;d++){mods[d]+=sources[r]*clip(s->p[34+r*7+d]+s->matrixmod[r*7+d],-100,100)*.01f;}
memset(s->matrixmod,0,sizeof(s->matrixmod));for(int a=0;a<3;a++){int target=(int)s->p[31+a],id=-1;if(target<18)id=targetids[target];else if(target<53){id=34+target-18;s->matrixmod[target-18]=mods[a+4]*100;}if(id>=0){s->modparam[id]=mods[a+4]*(defs[id].hi-defs[id].lo)*.5f;ep[id]=clip(s->p[id]+s->modparam[id],defs[id].lo,defs[id].hi);}}
#define E(k) ep[idx(k)]
s->active_model=(int)clip(ep[3],0,15);float mix=0,envsum=0;for(int i=0;i<N;i++){Voice*v=s->v+i;if(!v->stage)continue;float att=1/(SR*seconds(ep[10])),dec=1/(SR*seconds(ep[11]));if(v->stage==1){v->env+=att;if(v->env>=1){v->env=1;v->stage=2;}}else if(v->stage==2){v->env-=dec;if(v->env<=ep[12]){v->env=ep[12];v->stage=3;}}else if(v->stage==3){v->env=ep[12];}else if(v->stage==4){v->env-=dec;if(v->env<=0){v->env=0;v->stage=0;continue;}}float hz=440*powf(2,(v->note-69+12*ep[30]+s->bend*ep[29]+mods[0]*24)/12.f);hz=clip(hz,15,15000);float glide=ep[7];v->freq+=(hz-v->freq)*(glide<.001?1:1/(1+SR*seconds(glide)));float y=osc(s,v,(int)clip(ep[3],0,15),v->freq,clip(ep[4]+mods[1]*.5f,0,1),clip(ep[5]+mods[2]*.5f,0,1),ep[6]);mix+=y*v->vel*(ep[14]>.5?v->env:(v->stage==4?v->env:1));envsum=fmaxf(envsum,v->env);}s->env=envsum;
float cutoff=clip(ep[8]+mods[3]*.5f+envsum*(ep[13]-s->p[13]),0,1),hz=cutoff_hz(cutoff),g=tanf(PI*hz/SR),k=2-1.9f*ep[9];float a1=1/(1+g*(g+k)),v1=(s->band+g*(mix*.25f-s->low))*a1,v2=s->low+g*v1;s->band=2*v1-s->band;s->low=2*v2-s->low;float y=ep[15]<.5?v2:ep[15]<1.5?v1:mix*.25f-k*v1-v2;/* Remove DC before the final gain; keep normal signals linear and limit peaks only. */
float clean=y-s->dc_input+.99715f*s->dc;s->dc_input=y;s->dc=clean;y=output_limit(clean*1.6f)*ep[1];if(!isfinite(y)){y=0;s->low=s->band=s->dc=s->dc_input=0;}int16_t q=(int16_t)(clip(y,-1,1)*30000);out[2*f]=out[2*f+1]=q;
#undef E
}}
static void song_pos(void*o,double ppq,double bpm,int playing,int frames){(void)frames;Synth*s=o;if(bpm>0&&bpm<1000)s->bpm=bpm;if(playing&&(!s->transport_playing||ppq<s->transport_ppq-.01))rhythm_reset(s);s->transport_seen=1;s->transport_playing=playing;s->transport_ppq=ppq;}
const mpc_engine_t *mpc_engine(void){static const mpc_engine_t api={create,destroy,midi,set_param,get_param,render,NULL,song_pos};return &api;}
