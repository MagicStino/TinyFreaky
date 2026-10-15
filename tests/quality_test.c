/* SPDX-License-Identifier: MIT — exercise the complete signal path at parameter extremes. */
#include "../src/engine.c"
#include <assert.h>
int main(void){
 Synth*s=create(NULL);assert(s);int16_t audio[256];
 for(int model=0;model<16;model++)for(int mode=0;mode<3;mode++)for(int corner=0;corner<4;corner++){
  preset(s,0);silence(s);s->p[3]=model;s->p[15]=mode;s->p[8]=corner==0?0:corner==1?.5f:1;s->p[9]=corner==3?1:0;
  s->p[4]=corner&1;s->p[5]=(corner>>1)&1;s->p[6]=corner&1;s->p[1]=1;s->p[10]=0;s->p[12]=1;
  for(int note=0;note<8;note++)noteon(s,36+note*11,127);
  for(int block=0;block<32;block++){
   render(s,audio,128);assert(isfinite(s->low)&&isfinite(s->band)&&isfinite(s->dc));
   for(int i=0;i<256;i++)assert(abs(audio[i])<=30000);
  }
 }
 // Allocation keeps all eight held notes, steals the oldest on note nine, and releases every voice.
 preset(s,0);silence(s);for(int i=0;i<8;i++)noteon(s,60+i,100);
 for(int i=0;i<8;i++)assert(s->v[i].held&&s->v[i].note==60+i);
 char hint[128];get_param(s,"screen_hint",hint,sizeof hint);assert(strstr(hint,"8 VOICES"));
 noteon(s,80,100);assert(s->v[0].note==80);for(int i=1;i<8;i++)assert(s->v[i].note==60+i);
 for(int i=0;i<8;i++){unsigned char off[]={0x80,(unsigned char)s->v[i].note,0};voice_midi(s,off,3);}
 for(int i=0;i<8;i++)assert(!s->v[i].held&&s->v[i].stage==4);
 preset(s,0);silence(s);s->p[2]=0;noteon(s,60,100);noteon(s,64,100);
 assert(s->v[0].note==64);for(int i=1;i<8;i++)assert(!s->v[i].stage);
 // An asymmetric pulse must settle around zero, rather than consume output headroom with DC.
 preset(s,0);silence(s);s->p[4]=1;s->p[5]=.9;s->p[6]=0;s->p[8]=1;s->p[9]=0;s->p[10]=0;s->p[12]=1;noteon(s,69,127);
 double sum=0,energy=0;int count=0;
 for(int block=0;block<400;block++){render(s,audio,128);if(block>=200)for(int i=0;i<256;i+=2){double x=audio[i]/30000.;sum+=x;energy+=x*x;count++;}}
 assert(fabs(sum/count)<.002);assert(energy/count>.001);
 // SuperWave's two side voices must accumulate different phases when detune is raised.
 preset(s,0);silence(s);s->p[3]=1;s->p[5]=1;noteon(s,69,100);render(s,audio,128);
 assert(fabs(s->v[0].detune[0]-s->v[0].detune[1])>.01);
 // Capture publishes at 10 Hz with bounded quantized values, then returns to a flat line.
 preset(s,0);silence(s);noteon(s,69,100);char rev[32];get_param(s,"scope_rev",rev,sizeof rev);unsigned before=strtoul(rev,0,10);
 for(int b=0;b<35;b++)render(s,audio,128);
 assert(s->scope_rev==before+1);int moved=0;for(int i=0;i<35;i++)assert(s->visual[i]<= (i==34?3:i==33?152:16));for(int i=0;i<32;i++)if(s->visual[i]!=8)moved++;assert(moved>8);
 silence(s);for(int i=0;i<32;i++)assert(s->visual[i]==8);assert(s->visual[34]==0);
 // Aliasing regression: exact multiples of the 10Hz refresh must show a full LFO sweep.
 for(int shape=0;shape<4;shape++)for(int hz=10;hz<=40;hz+=10){preset(s,0);silence(s);s->p[16]=shape;s->p[17]=hz;s->p[18]=0;render(s,audio,128);for(int b=1;b<35;b++)render(s,audio,128);assert(s->visual[33]==136);}
 destroy(s);puts("quality: 16 engines x 3 filter modes x 4 extremes, eight-voice allocation/stealing/release, mono compatibility, polyphonic headroom, DC rejection and real detuning PASS");
}
