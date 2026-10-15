// SPDX-License-Identifier: MIT
#include "plugin.h"
#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <cstring>
namespace Tinyfreaky {
static const char* options[]={
#include "options.inc"
};
tresult PLUGIN_API Controller::initialize(FUnknown*c){auto result=EditController::initialize(c);if(result!=kResultOk)return result;for(int i=0;i<Count;i++){auto&d=definitions[i];String128 name{};UString(name,128).fromAscii(d.name);int32 flags=ParameterInfo::kCanAutomate;if(i==0)flags|=ParameterInfo::kIsProgramChange;if(options[i][0]||i==0){auto* list=new StringListParameter(name,i,STR16(""),flags|ParameterInfo::kIsList);if(i==0){const auto* api=mpc_engine();void* engine=api->create(std::getenv("TINYFREAKY_PRESETS"));char countText[32]{};int bankCount=8;if(engine&&api->get_param(engine,"bank_count",countText,sizeof(countText))>0)bankCount=std::atoi(countText);for(int program=0;program<=int(d.hi);program++){char key[32],label[128];std::snprintf(key,sizeof(key),"preset:%d",program);if(program>=bankCount)std::snprintf(label,sizeof(label),"Unavailable slot %d",program+1);else if(!engine||api->get_param(engine,key,label,sizeof(label))<=0)std::snprintf(label,sizeof(label),"Preset %d",program+1);String128 title{};UString(title,128).fromAscii(label);list->appendString(title);}if(engine)api->destroy(engine);}else{std::string labels(options[i]);size_t start=0;do{size_t end=labels.find('\t',start);String128 title{};UString(title,128).fromAscii(labels.substr(start,end-start).c_str());list->appendString(title);if(end==std::string::npos)break;start=end+1;}while(true);}list->setNormalized((d.initial-d.lo)/(d.hi-d.lo));parameters.addParameter(list);}else parameters.addParameter(new RangeParameter(name,i,STR16(""),d.lo,d.hi,d.initial,d.steps,flags));}for(int c=0;c<130;c++){String128 title{};char text[32];std::snprintf(text,sizeof(text),"MIDI %d",c);UString(title,128).fromAscii(text);parameters.addParameter(title,STR16(""),c==128?16383:127,c==128?.5:0,ParameterInfo::kCanAutomate|ParameterInfo::kIsHidden,MidiBase+c);}return kResultOk;}
tresult PLUGIN_API Controller::setComponentState(IBStream*s){char magic[4];uint32 count;int32 got;if(!s||s->read(magic,4,&got)!=kResultOk||got!=4||std::memcmp(magic,"MFV3",4))return kResultFalse;if(s->read(&count,4,&got)!=kResultOk||got!=4||count!=Count)return kResultFalse;for(uint32 i=0;i<count;i++){double v;if(s->read(&v,8,&got)!=kResultOk||got!=8||!std::isfinite(v))return kResultFalse;setParamNormalized(i,v);}return kResultOk;}
tresult PLUGIN_API Controller::getMidiControllerAssignment(int32 bus,int16 channel,CtrlNumber control,ParamID& id){if(bus!=0||channel<0||channel>15||control<0||control>129)return kResultFalse;id=MidiBase+control;return kResultTrue;}
}
