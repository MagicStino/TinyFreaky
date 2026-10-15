#include "../src/engine.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void){const mpc_engine_t*e=mpc_engine();void*s=e->create(NULL);char name[256],value[64],status[256];e->get_param(s,"bank_count",value,sizeof value);assert(atoi(value)==456);int unavailable=0,mapped=0;for(int i=8;i<456;i++){char key[32];snprintf(key,sizeof key,"preset:%d",i);e->get_param(s,key,name,sizeof name);assert(name[0]);snprintf(value,sizeof value,"%d",i);e->set_param(s,"preset",value);e->get_param(s,"status",status,sizeof status);if(!strncmp(name,"[Unavailable]",13)){unavailable++;assert(strstr(status,"muted"));unsigned char note[]={0x90,60,100};e->midi(s,note,3);short audio[256];e->render(s,audio,128);for(int n=0;n<256;n++)assert(audio[n]==0);}else{mapped++;assert(strncmp(name,"~ ",2));}}assert(unavailable==114&&mapped==334);e->destroy(s);puts("factory: all 448 originals indexed, 334 partial translations, 114 unavailable engines muted PASS");}
