#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void *tf_plaits_create(void);
void tf_plaits_destroy(void*);
void tf_plaits_reset(void*);
float tf_plaits_sample(void*,int,float,float,float,float,int,float);
#ifdef __cplusplus
}
#endif
