// SPDX-License-Identifier: MIT
#include "plugin.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/common/memorystream.h"
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace Microfreeze;
static void require(bool ok,const char* message){if(!ok){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
static std::vector<float> render(double rate,int block,int offset){Processor p;require(p.initialize(nullptr)==kResultOk,"initialize");ProcessSetup setup{};setup.symbolicSampleSize=kSample32;setup.sampleRate=rate;setup.maxSamplesPerBlock=block;require(p.setupProcessing(setup)==kResultOk,"setup");p.setActive(true);p.setProcessing(true);std::vector<float> out;for(int pos=0;pos<2048;){int frames=std::min(block,2048-pos);std::vector<float> left(frames),right(frames);float* channels[]={left.data(),right.data()};AudioBusBuffers bus{};bus.numChannels=2;bus.channelBuffers32=channels;EventList events;Event e{};e.type=Event::kNoteOnEvent;e.noteOn.pitch=60;e.noteOn.velocity=1.;e.noteOn.channel=0;e.sampleOffset=offset-pos;if(pos<=offset&&offset<pos+frames)events.addEvent(e);ProcessData data{};data.symbolicSampleSize=kSample32;data.numSamples=frames;data.numOutputs=1;data.outputs=&bus;data.inputEvents=&events;require(p.process(data)==kResultOk,"process");for(float v:left){require(std::isfinite(v)&&std::abs(v)<=1.01f,"finite bounded output");out.push_back(v);}pos+=frames;}MemoryStream saved;require(p.getState(&saved)==kResultOk,"save state");saved.seek(0,IBStream::kIBSeekSet,nullptr);require(p.setState(&saved)==kResultOk,"restore state");MemoryStream restored;require(p.getState(&restored)==kResultOk,"save restored state");require(saved.getSize()==restored.getSize()&&!std::memcmp(saved.getData(),restored.getData(),saved.getSize()),"state roundtrip preserves parameter values");p.setProcessing(false);p.setActive(false);p.terminate();return out;}
int main(){for(double rate:{44100.,48000.,96000.}){auto a=render(rate,64,0),b=render(rate,257,0);double energy=0;for(size_t i=0;i<a.size();i++){require(std::abs(a[i]-b[i])<1e-7,"resampling independent of host block partition");energy+=a[i]*a[i];}require(energy>1e-6,"MIDI produces audio");}auto delayed=render(44100,512,113);for(int i=0;i<113;i++)require(delayed[i]==0,"no sound before note offset");std::puts("Desktop smoke: rates 44.1/48/96 kHz, block partition, MIDI offset, state passed");}
