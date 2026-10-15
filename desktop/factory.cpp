// SPDX-License-Identifier: MIT
#include "plugin.h"
#include "public.sdk/source/main/pluginfactory.h"
using namespace Steinberg::Vst;
#define stringPluginName "Microfreeze"
BEGIN_FACTORY_DEF("MagicStino", "https://github.com/MagicStino/microfreeze", "")
 DEF_CLASS2(INLINE_UID_FROM_FUID(Microfreeze::ProcessorID),PClassInfo::kManyInstances,kVstAudioEffectClass,stringPluginName,Vst::kDistributable,"Instrument|Synth","0.1.0",kVstVersionString,Microfreeze::Processor::createInstance)
 DEF_CLASS2(INLINE_UID_FROM_FUID(Microfreeze::ControllerID),PClassInfo::kManyInstances,kVstComponentControllerClass,"Microfreeze Controller",0,"","0.1.0",kVstVersionString,Microfreeze::Controller::createInstance)
END_FACTORY
