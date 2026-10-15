// SPDX-License-Identifier: MIT
#pragma once
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "pluginterfaces/base/ustring.h"
#include <array>
#include <mutex>
extern "C" {
#include "../src/engine.h"
}
namespace Tinyfreaky {
using namespace Steinberg;
using namespace Steinberg::Vst;
static const FUID ProcessorID(0x71CE65D1,0x38424B2A,0x95113928,0xCE77AB01);
static const FUID ControllerID(0x71CE65D1,0x38424B2A,0x95113928,0xCE77AB02);
struct Definition { const char* key; const char* name; double lo,hi,initial; int steps; };
static const Definition definitions[]={
#include "parameters.inc"
};
constexpr int Count=sizeof(definitions)/sizeof(definitions[0]);
constexpr ParamID MidiBase=10000;
class Processor final: public AudioEffect {
public:
 Processor(); ~Processor() override;
 static FUnknown* createInstance(void*){return static_cast<IAudioProcessor*>(new Processor);}
 tresult PLUGIN_API initialize(FUnknown*) override;
 tresult PLUGIN_API setupProcessing(ProcessSetup&) override;
 tresult PLUGIN_API setBusArrangements(SpeakerArrangement*,int32,SpeakerArrangement*,int32) override;
 tresult PLUGIN_API canProcessSampleSize(int32) override;
 tresult PLUGIN_API setActive(TBool) override;
 tresult PLUGIN_API process(ProcessData&) override;
 tresult PLUGIN_API setState(IBStream*) override;
 tresult PLUGIN_API getState(IBStream*) override;
private:
 std::mutex engineMutex;
 const mpc_engine_t* api; void* instance=nullptr;
 std::array<double,Count> normalized{};
 bool presetChanged=false;
 double ratio=1,phase=0; float before[2]{},current[2]{};
 struct Action {int offset; int type; int index; double value; Event event; int order;};
 std::array<Action,8192> actions{};
 void parameter(int,double); void note(const Event&); void resetAudio();
};
class Controller final: public EditController, public IMidiMapping, public VSTGUI::VST3EditorDelegate {
public:
 static FUnknown* createInstance(void*){return static_cast<IEditController*>(new Controller);}
 IPlugView* PLUGIN_API createView(FIDString) override;
 VSTGUI::CView* createCustomView(VSTGUI::UTF8StringPtr,const VSTGUI::UIAttributes&,const VSTGUI::IUIDescription*,VSTGUI::VST3Editor*) override;
 tresult PLUGIN_API initialize(FUnknown*) override;
 tresult PLUGIN_API setComponentState(IBStream*) override;
 tresult PLUGIN_API getMidiControllerAssignment(int32,int16,CtrlNumber,ParamID&) override;
 OBJ_METHODS(Controller,EditController)
 DEFINE_INTERFACES
  DEF_INTERFACE(IMidiMapping)
 END_DEFINE_INTERFACES(EditController)
 REFCOUNT_METHODS(EditController)
};
}
