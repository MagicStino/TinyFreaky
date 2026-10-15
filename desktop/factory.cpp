// SPDX-License-Identifier: MIT
#include "plugin.h"
#include "public.sdk/source/main/pluginfactory.h"
using namespace Steinberg::Vst;
#define stringPluginName "Tinyfreaky"
BEGIN_FACTORY_DEF("MagicStino", "https://github.com/MagicStino/microfreeze", "")
 DEF_CLASS2(INLINE_UID_FROM_FUID(Tinyfreaky::ProcessorID),PClassInfo::kManyInstances,kVstAudioEffectClass,stringPluginName,Vst::kDistributable,"Instrument|Synth","0.2.0",kVstVersionString,Tinyfreaky::Processor::createInstance)
 DEF_CLASS2(INLINE_UID_FROM_FUID(Tinyfreaky::ControllerID),PClassInfo::kManyInstances,kVstComponentControllerClass,"Tinyfreaky Controller",0,"","0.2.0",kVstVersionString,Tinyfreaky::Controller::createInstance)
END_FACTORY
