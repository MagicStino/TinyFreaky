// SPDX-License-Identifier: MIT
// Every playable factory slot must provide a usable body somewhere on the keyboard.
#include "../src/engine.c"
#include <assert.h>
int main(void){Synth*s=create(NULL);assert(s);int16_t audio[882];int tested=0;
 for(int slot=0;slot<456;slot++){
  if(slot>=8&&!s->bank[slot-8].playable)continue;
  int usable=0;
  for(int octave=0;octave<3&&!usable;octave++){
   char value[20];snprintf(value,sizeof value,"%d",slot);set_param(s,"preset",value);set_param(s,"arp_on","0");set_param(s,"seq_on","0");noteon(s,48+12*octave,100);
   int audible=0,peak=0;
   for(int window=0;window<300;window++){render(s,audio,441);double energy=0;for(int i=0;i<882;i+=2){int x=abs(audio[i]);if(x>peak)peak=x;energy+=(double)x*x;}if(energy/441>3600)audible++;}
   usable=audible>=10&&peak>=300;
  }
  if(!usable){char key[32],name[128];snprintf(key,sizeof key,"preset:%d",slot);get_param(s,key,name,sizeof name);fprintf(stderr,"Unusable factory sound: %d %s\n",slot,name);}
  assert(usable);tested++;
 }
 assert(tested==342);destroy(s);puts("sound: all 342 playable slots have >=100ms audible body and usable peak across C3/C4/C5 PASS");
}
