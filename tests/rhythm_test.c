#include "../src/engine.c"
#include <assert.h>
#include <stdio.h>
static void press(Synth*s,int note){unsigned char m[]={0x90,(unsigned char)note,100};midi(s,m,3);}
static int next(Synth*s,int*elapsed){int before=s->rhythm_step;*elapsed=0;while(s->rhythm_step==before&&*elapsed<100000){rhythm_tick(s);(*elapsed)++;}return s->last_rhythm_note;}
int main(void){Synth*s=create(NULL);int frames;set_param(s,"arp_on","1");press(s,67);press(s,60);press(s,64);
 assert(next(s,&frames)==60);assert(next(s,&frames)==64);assert(frames>=5512&&frames<=5513);assert(next(s,&frames)==67);
 set_param(s,"arp_mode","1");rhythm_reset(s);assert(next(s,&frames)==67);assert(next(s,&frames)==60);assert(next(s,&frames)==64);
 set_param(s,"swing","75");rhythm_reset(s);next(s,&frames);next(s,&frames);int long_step=frames;next(s,&frames);assert(long_step>=8268&&long_step<=8270);assert(frames>=2755&&frames<=2757);assert(abs(long_step+frames-11025)<=1);
 set_param(s,"all_notes_off","1");set_param(s,"arp_on","0");set_param(s,"seq_length","4");set_param(s,"seq_note_0","60");set_param(s,"seq_note_1","-1");set_param(s,"seq_note_2","67");set_param(s,"seq_note_3","72");set_param(s,"seq_on","1");
 assert(next(s,&frames)==60);assert(next(s,&frames)==-1);assert(next(s,&frames)==67);assert(next(s,&frames)==72);assert(next(s,&frames)==60);
 set_param(s,"rhythm_sync","1");rhythm_reset(s);for(int i=0;i<10000;i++)rhythm_tick(s);assert(s->rhythm_step==0);
 song_pos(s,0,120,1,128);assert(next(s,&frames)==60);song_pos(s,1,120,0,128);int old=s->rhythm_step;for(int i=0;i<10000;i++)rhythm_tick(s);assert(s->rhythm_step==old&&s->last_rhythm_note==-1);
 set_param(s,"seq_note_63","127");char state[16384],copy[16384];get_param(s,"state",state,sizeof state);assert(!strncmp(state,"MFZ2;",5));Synth*t=create(NULL);set_param(t,"state",state);get_param(t,"state",copy,sizeof copy);assert(!strcmp(state,copy));
 char legacy[16384];strcpy(legacy,state);legacy[3]='1';char*p=legacy+5;for(int i=0;i<70;i++){p=strchr(p,';');assert(p);p++;}*p=0;set_param(t,"state",legacy);assert(t->p[R_SEQ_ON]==0&&t->p[R_SEQ_NOTE_0]==60&&t->p[R_SWING]==50);
 destroy(t);destroy(s);puts("rhythm: up/order, rate, swing timing, rests/64 steps, host start/stop, save and legacy migration PASS");}
