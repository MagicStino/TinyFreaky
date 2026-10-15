// SPDX-License-Identifier: MIT
// Extra host-side regression: display telemetry must actually reach the editor.
#include "../desktop/plugin.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace Tinyfreaky;
static void require(bool ok,const char* message){if(!ok){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){
 Processor p;require(p.initialize(nullptr)==kResultOk,"mascot initialize");
 ProcessSetup setup{};setup.symbolicSampleSize=kSample32;setup.sampleRate=48000;setup.maxSamplesPerBlock=512;require(p.setupProcessing(setup)==kResultOk,"mascot setup");p.setActive(true);p.setProcessing(true);
 float left[512]{},right[512]{};float* channels[]={left,right};AudioBusBuffers bus{};bus.numChannels=2;bus.channelBuffers32=channels;ParameterChanges output(VisualCount);bool seen[541]{};int count=0;
 for(int block=0;block<400;block++){EventList input;if(block==0){Event e{};e.type=Event::kNoteOnEvent;e.noteOn.pitch=84;e.noteOn.velocity=.9;input.addEvent(e);}output.clearQueue();ProcessData data{};data.symbolicSampleSize=kSample32;data.numSamples=512;data.numOutputs=1;data.outputs=&bus;data.inputEvents=&input;data.outputParameterChanges=&output;require(p.process(data)==kResultOk,"mascot process");for(int i=0;i<output.getParameterCount();i++){auto*q=output.getParameterData(i);if(q->getParameterId()!=VisualBase+35)continue;int32 offset;ParamValue value;require(q->getPoint(q->getPointCount()-1,offset,value)==kResultOk,"mascot queue point");require(value>=0&&value<=1,"normalized pose bounds");seen[(int)std::lround(value*540)]=true;}}
 for(int i=1;i<=540;i++)count+=seen[i];require(count>40,"held note delivers moving poses through VST3 output queue");p.setProcessing(false);p.setActive(false);p.terminate();std::printf("VST3 mascot host: %d distinct held-note poses delivered at 48 kHz PASS\n",count);
}
