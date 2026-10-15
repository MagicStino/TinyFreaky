// SPDX-License-Identifier: MIT
#include "../src/engine.c"
#include <assert.h>
static void event(Synth*s,int status,int note,int vel){unsigned char m[]={(unsigned char)status,(unsigned char)note,(unsigned char)vel};midi(s,m,3);}
static int held(Synth*s,int note){int n=0;for(int i=0;i<N;i++)n+=s->v[i].stage&&s->v[i].held&&s->v[i].note==note;return n;}
int main(void){Synth*s=create(NULL);assert(s);s->p[2]=1;
 event(s,0x90,60,100);event(s,0x90,60,100);event(s,0x80,60,0);assert(held(s,60)==1);
 event(s,0x80,60,0);assert(held(s,60)==0);
 silence(s);event(s,0x90,60,100);event(s,0x91,60,100);event(s,0x80,60,0);assert(held(s,60)==1);
 silence(s);event(s,0x90,60,100);for(int n=61;n<68;n++)event(s,0x90,n,100);event(s,0x90,60,100);event(s,0x80,60,0);assert(held(s,60)==1);
 event(s,0x80,60,0);assert(held(s,60)==0);
 silence(s);event(s,0x90,60,100);event(s,0x91,64,100);event(s,0xb0,123,0);assert(held(s,64)==1&&held(s,60)==0);
 event(s,0xb1,120,0);for(int i=0;i<N;i++)assert(!s->v[i].stage||s->v[i].note!=64);
 /* A released newer voice must be reclaimed before the oldest held note. */
 silence(s);for(int n=60;n<68;n++)event(s,0x90,n,100);event(s,0x80,67,0);event(s,0x90,72,100);assert(held(s,60)==1&&held(s,72)==1);
 /* Host start/loop callbacks must not release externally played chords. */
 silence(s);event(s,0x90,55,100);event(s,0x90,71,100);event(s,0x90,74,100);
 song_pos(s,0,78,1,128);assert(held(s,55)==1&&held(s,71)==1&&held(s,74)==1);
 song_pos(s,31,78,1,128);song_pos(s,0,78,1,128);assert(held(s,55)==1&&held(s,71)==1&&held(s,74)==1);
 /* Reported keys patches must sustain all four voices across a clip-position jump. */
 for(int patch=0;patch<2;patch++){char slot[16];sprintf(slot,"%d",patch?21:63);set_param(s,"preset",slot);set_param(s,"paraphonic","1");
  const int chord[]={48,63,67,70};for(int n=0;n<4;n++)event(s,0x90,chord[n],100);
  short audio[256];for(int b=0;b<35;b++)render(s,audio,128);
  song_pos(s,16.1,78,1,128);song_pos(s,0,78,1,128);song_pos(s,-1,0,1,128);
  for(int n=0;n<4;n++)assert(held(s,chord[n])==1);
  double energy=0;for(int b=0;b<700;b++){render(s,audio,128);if(b>=350)for(int f=0;f<256;f+=2)energy+=(double)audio[f]*audio[f];}
  assert(sqrt(energy/(350*128))/30000.>.01);
 }
 destroy(s);puts("overlap: repeated pitches, shared pitches across channels, stolen-note releases and channel all-notes-off, release-tail priority and transport-loop chords PASS");}
