// SPDX-License-Identifier: MIT
// Adapter to unmodified upstream MIT engines. This does not claim other implementations.
#include "plaits_bridge.h"
#include "plaits/dsp/engine/virtual_analog_engine.h"
#include "plaits/dsp/engine/waveshaping_engine.h"
#include "plaits/dsp/engine/fm_engine.h"
#include "plaits/dsp/engine/grain_engine.h"
#include "plaits/dsp/engine/chord_engine.h"
#include "plaits/dsp/engine/speech_engine.h"
#include "plaits/dsp/engine/modal_engine.h"
#include "plaits/dsp/engine/wavetable_engine.h"
#include "plaits/dsp/engine/additive_engine.h"
#include "plaits/dsp/engine/string_engine.h"
#include "plaits/dsp/engine/noise_engine.h"
#include <cmath>
#include <cstring>
#include <new>
#include <mutex>
namespace {
std::mutex lock;
struct Voice {
 alignas(16) unsigned char memory[65536]{};
 stmlib::BufferAllocator allocator;
 plaits::VirtualAnalogEngine va;plaits::WaveshapingEngine ws;plaits::FMEngine fm;
 plaits::GrainEngine grain;plaits::ChordEngine chord;plaits::SpeechEngine speech;
 plaits::ModalEngine modal;plaits::WavetableEngine table;plaits::AdditiveEngine harmonic;
 plaits::StringEngine string;plaits::NoiseEngine noise;
 plaits::Engine* active=nullptr;
 int model=-1,pos=24;bool trigger=true;
 float out[24]{},aux[24]{};double phase=1;float before=0,current=0;
 plaits::Engine* choose(int m){switch(m){case 2:return &table;case 3:return &harmonic;case 4:return &string;case 5:return &va;case 6:return &ws;case 7:return &fm;case 8:return &grain;case 9:return &chord;case 10:return &speech;case 11:return &modal;case 12:return &noise;default:return nullptr;}}
 void reset(){trigger=true;pos=24;phase=1;before=current=0;if(active)active->Reset();}
};
}
extern "C" void* tf_plaits_create(){return new(std::nothrow) Voice;}
extern "C" void tf_plaits_destroy(void*p){delete static_cast<Voice*>(p);}
extern "C" void tf_plaits_reset(void*p){if(!p)return;std::lock_guard<std::mutex>g(lock);static_cast<Voice*>(p)->reset();}
extern "C" float tf_plaits_sample(void*p,int model,float hz,float wave,float timbre,float shape,int gate,float accent){
 if(!p||!std::isfinite(hz)||hz<=0)return 0;
 auto&v=*static_cast<Voice*>(p);
 if(model!=v.model){std::lock_guard<std::mutex>g(lock);v.active=v.choose(model);v.model=model;if(!v.active)return 0;v.allocator.Init(v.memory,sizeof v.memory);v.active->Init(&v.allocator);v.active->LoadUserData(nullptr);v.reset();}
 if(!v.active)return 0;
 // Upstream normalized frequencies use the measured 47872.34 Hz Plaits clock.
 // Resampling preserves its pitch and block timing on the 44100 Hz Force ABI.
 v.phase+=47872.34/44100.;
 while(v.phase>=1){
  v.before=v.current;
  if(v.pos>=24){plaits::EngineParameters params{};params.note=69.f+12.f*std::log2(hz/440.f);params.harmonics=wave;params.timbre=timbre;params.morph=shape;params.accent=accent;params.trigger=(gate?plaits::TRIGGER_HIGH:plaits::TRIGGER_LOW)|(v.trigger?plaits::TRIGGER_RISING_EDGE:0);bool enveloped=false;{std::lock_guard<std::mutex>g(lock);v.active->Render(params,v.out,v.aux,24,&enveloped);}v.pos=0;v.trigger=false;}
  v.current=v.out[v.pos++];v.phase-=1;
 }
 float y=v.before+(v.current-v.before)*v.phase;return std::isfinite(y)?y:0;
}
