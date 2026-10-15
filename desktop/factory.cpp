// SPDX-License-Identifier: MIT
#include "plugin.h"
#include "public.sdk/source/main/pluginfactory.h"
using namespace Steinberg::Vst;
#define stringPluginName "Tinyfreaky"
BEGIN_FACTORY_DEF("MagicStino", "https://github.com/MagicStino/TinyFreaky", "")
 DEF_CLASS2(INLINE_UID_FROM_FUID(Tinyfreaky::ProcessorID),PClassInfo::kManyInstances,kVstAudioEffectClass,stringPluginName,Vst::kDistributable,"Instrument|Synth","0.3.6",kVstVersionString,Tinyfreaky::Processor::createInstance)
 DEF_CLASS2(INLINE_UID_FROM_FUID(Tinyfreaky::ControllerID),PClassInfo::kManyInstances,kVstComponentControllerClass,"Tinyfreaky Controller",0,"","0.3.6",kVstVersionString,Tinyfreaky::Controller::createInstance)
END_FACTORY
