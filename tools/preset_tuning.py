"""Playback calibration for the shipped TinyFreaky bank, not user patches."""
import math
def tune(values):
 v=list(values)
 if int(v[3])==12:
  # Avoid stacking sub-bass VCF settings on an already filtered noise source.
  if v[15]<.5:v[8]=.28+.72*math.sqrt(max(0,min(1,v[8])))
  elif v[15]<1.5:v[8]=.28+.25*v[8]
  else:
   v[8]=.14+.28*v[8]
   if v[44]>0:v[44]=-v[44];v[13]=(v[44]+100)*.005
 if v[12]<.15:
  v[11]=max(.28,v[11])
 if int(v[3])==4:v[6]=max(.35,v[6])
 if int(v[3])==11:
  v[6]=max(.25,v[6])
  if v[15]>=1.5 and v[11]<.1:
   v[8]=min(.22,v[8]);v[11]=.28
 return v
