/* SPDX-License-Identifier: MIT — exercise the complete signal path at parameter extremes. */
#include "../src/engine.c"
#include <assert.h>
int main(void){
 Synth*s=create(NULL);assert(s);int16_t audio[256];
 for(int model=0;model<16;model++)for(int mode=0;mode<3;mode++)for(int corner=0;corner<4;corner++){
  preset(s,0);silence(s);s->p[3]=model;s->p[15]=mode;s->p[8]=corner==0?0:corner==1?.5f:1;s->p[9]=corner==3?1:0;
  s->p[4]=corner&1;s->p[5]=(corner>>1)&1;s->p[6]=corner&1;s->p[1]=1;s->p[10]=0;s->p[12]=1;
  for(int note=0;note<4;note++)noteon(s,36+note*19,127);
  for(int block=0;block<32;block++){
   render(s,audio,128);assert(isfinite(s->low)&&isfinite(s->band)&&isfinite(s->dc));
   for(int i=0;i<256;i++)assert(abs(audio[i])<=30000);
  }
 }
 // An asymmetric pulse must settle around zero, rather than consume output headroom with DC.
 preset(s,0);silence(s);s->p[4]=1;s->p[5]=.9;s->p[6]=0;s->p[8]=1;s->p[9]=0;s->p[10]=0;s->p[12]=1;noteon(s,69,127);
 double sum=0,energy=0;int count=0;
 for(int block=0;block<400;block++){render(s,audio,128);if(block>=200)for(int i=0;i<256;i+=2){double x=audio[i]/30000.;sum+=x;energy+=x*x;count++;}}
 assert(fabs(sum/count)<.002);assert(energy/count>.001);
 // SuperWave's two side voices must accumulate different phases when detune is raised.
 preset(s,0);silence(s);s->p[3]=1;s->p[5]=1;noteon(s,69,100);render(s,audio,128);
 assert(fabs(s->v[0].detune[0]-s->v[0].detune[1])>.01);
 destroy(s);puts("quality: 16 engines x 3 filter modes x 4 extremes, polyphonic headroom, DC rejection and real detuning PASS");
}
