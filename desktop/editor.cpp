// SPDX-License-Identifier: MIT
#include "plugin.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "vstgui4/vstgui/lib/cdrawcontext.h"
#include "vstgui4/vstgui/lib/cfont.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>
namespace Tinyfreaky {
using namespace VSTGUI;
struct Knob {const char* key;const char* label;double x,y,r;const char* color;};
struct Group {const char* label;double x,y,w,h;};
static const Knob panel[]={
#include "panel.inc"
};
static const Group groups[]={
#include "groups.inc"
};
static const CColor bg(16,43,37),surface(22,56,47),white(242,239,223),orange(241,138,60),green(184,219,145),muted(162,181,167);
static int indexOf(const char* key){for(int i=0;i<Count;i++)if(!std::strcmp(key,definitions[i].key))return i;return -1;}
class Panel final:public CView {
 Controller* controller;int page=0,editing=-1;double startY=0,startValue=0;std::vector<Knob> items;
 void text(CDrawContext* d,const char* s,double x,double y,double w,double h,int size=13,CColor color=white){d->setFont(kNormalFont,size);d->setFontColor(color);d->drawString(s,CRect(x,y,x+w,y+h),kCenterText);}
 void refreshItems(){items.clear();if(page==0)items.assign(std::begin(panel),std::end(panel));else if(page==1){for(int r=0;r<5;r++)for(int c=0;c<7;c++){static const char* keys[35]={
#define M(r,c) "matrix_" #r "_" #c
 M(0,0),M(0,1),M(0,2),M(0,3),M(0,4),M(0,5),M(0,6),M(1,0),M(1,1),M(1,2),M(1,3),M(1,4),M(1,5),M(1,6),M(2,0),M(2,1),M(2,2),M(2,3),M(2,4),M(2,5),M(2,6),M(3,0),M(3,1),M(3,2),M(3,3),M(3,4),M(3,5),M(3,6),M(4,0),M(4,1),M(4,2),M(4,3),M(4,4),M(4,5),M(4,6)
#undef M
 };items.push_back({keys[r*7+c],"",100.+c*180,135.+r*94,22,"green"});}}else{static const char* keys[]={"cyc_mode","cyc_rise_curve","cyc_fall_curve","amp_mod","tempo","bend_range","assign1","assign2","assign3","all_notes_off"};for(int i=0;i<10;i++)items.push_back({keys[i],definitions[indexOf(keys[i])].name,140.+i%5*245,200.+i/5*200,34,"white"});}}
 void finish(){if(editing>=0){controller->endEdit(editing);editing=-1;}}
public:
 Panel(Controller*c):CView(CRect(0,0,1280,628)),controller(c){refreshItems();setWantsIdle(true);}
 ~Panel()override{finish();}
 void onIdle()override{invalid();}
 void draw(CDrawContext*d)override{
  d->setDrawMode(kAntiAliasing);d->setFillColor(bg);d->drawRect(getViewSize(),kDrawFilled);
  text(d,"tinyfreaky",24,12,238,45,36);text(d,"EXPERIMENTAL HYBRID SYNTH",280,20,300,25,13,green);text(d,"MAGICSTINO / 0.2",1040,20,215,25,13,orange);
  if(page==0){for(auto&g:groups){d->setFillColor(surface);d->setFrameColor(muted);d->setLineWidth(.5);d->drawRect(CRect(g.x,g.y,g.x+g.w,g.y+g.h),kDrawFilledAndStroked);text(d,g.label,g.x+12,g.y+7,g.w-24,25,14,green);}
   d->setFillColor(CColor(9,25,16));d->drawRect(CRect(612,98,938,216),kDrawFilled);text(d,"TINYFREAKY",627,108,290,24,14,green);
   String128 title{};controller->getParamStringByValue(0,controller->getParamNormalized(0),title);char label[128]{};UString(title,128).toAscii(label,128);text(d,label,627,136,290,30,19);text(d,"Prototype engine / MIDI input",627,179,290,24,12,orange);
   const char* row[]={"CYC","ENV","LFO","PRESS","KEY"};for(int r=0;r<5;r++){text(d,row[r],30,116+r*21,55,18,10,muted);for(int c=0;c<7;c++){char key[24];std::snprintf(key,sizeof key,"matrix_%d_%d",r,c);double v=controller->getParamNormalized(indexOf(key));d->setFillColor(std::abs(v-.5)>.005?orange:CColor(73,107,83));d->drawEllipse(CRect(109+c*47,123+r*21,116+c*47,130+r*21),kDrawFilled);}}
   text(d,"CLICK MATRIX TO EDIT ROUTING",85,218,350,17,10,muted);
  }else if(page==1){const char* rows[]={"CYCLING ENV","ENVELOPE","LFO","PRESSURE","KEY / ARP"};const char* cols[]={"PITCH","WAVE","TIMBRE","CUTOFF","ASSIGN 1","ASSIGN 2","ASSIGN 3"};for(int c=0;c<7;c++)text(d,cols[c],30+c*180,75,140,22,13,green);for(int r=0;r<5;r++)text(d,rows[r],12,103+r*94,72,20,9,muted);}
  for(auto&k:items){int id=indexOf(k.key);double value=controller->getParamNormalized(id);CColor col=!std::strcmp(k.color,"orange")?orange:!std::strcmp(k.color,"green")?green:white;d->setFrameColor(col);d->setLineWidth(1);d->drawEllipse(CRect(k.x-k.r-5,k.y-k.r-5,k.x+k.r+5,k.y+k.r+5));d->setFillColor(col);d->drawEllipse(CRect(k.x-k.r,k.y-k.r,k.x+k.r,k.y+k.r),kDrawFilled);double a=(135+270*value)*3.141592653589793/180.;d->setFrameColor(bg);d->setLineWidth(3);d->drawLine(CPoint(k.x,k.y),CPoint(k.x+std::cos(a)*(k.r-6),k.y+std::sin(a)*(k.r-6)));text(d,k.label,k.x-73,k.y+k.r+7,146,20,11);String128 v{};controller->getParamStringByValue(id,value,v);char s[128]{};UString(v,128).toAscii(s,128);text(d,s,k.x-73,k.y+k.r+27,146,19,11,green);}
  for(int i=0;i<3;i++){d->setFillColor(i==page?orange:surface);d->drawRect(CRect(24+i*146,590,160+i*146,620),kDrawFilled);text(d,i==0?"PANEL":i==1?"MATRIX":"ADVANCED",24+i*146,592,136,26,12,i==page?bg:white);}text(d,"RESEARCH BUILD / SOUND EQUIVALENCE UNVERIFIED",720,590,534,30,11,muted);setDirty(false);
 }
 CMouseEventResult onMouseDown(CPoint&w,const CButtonState&b)override{if(!b.isLeftButton())return kMouseEventNotHandled;finish();if(w.y>=590&&w.x>=24&&w.x<452){page=std::min(2,int((w.x-24)/146));refreshItems();invalid();return kMouseDownEventHandledButDontNeedMovedOrUpEvents;}if(page==0&&w.x<454&&w.y>80&&w.y<238){page=1;refreshItems();invalid();return kMouseDownEventHandledButDontNeedMovedOrUpEvents;}for(auto&k:items)if(std::abs(w.x-k.x)<k.r+20&&std::abs(w.y-k.y)<k.r+20){editing=indexOf(k.key);startY=w.y;startValue=controller->getParamNormalized(editing);controller->beginEdit(editing);return kMouseEventHandled;}return kMouseEventNotHandled;}
 CMouseEventResult onMouseMoved(CPoint&w,const CButtonState&)override{if(editing<0)return kMouseEventNotHandled;double v=std::clamp(startValue+(startY-w.y)/180.,0.,1.);auto&def=definitions[editing];if(def.steps)v=std::round(v*def.steps)/def.steps;controller->setParamNormalized(editing,v);controller->performEdit(editing,v);invalid();return kMouseEventHandled;}
 CMouseEventResult onMouseUp(CPoint&,const CButtonState&)override{finish();return kMouseEventHandled;}
 CMouseEventResult onMouseCancel()override{finish();return kMouseEventHandled;}
};
CView* Controller::createCustomView(UTF8StringPtr name,const UIAttributes&,const IUIDescription*,VST3Editor*){return !std::strcmp(name,"TinyfreakyPanel")?new Panel(this):nullptr;}
IPlugView* PLUGIN_API Controller::createView(FIDString name){if(std::strcmp(name,ViewType::kEditor))return nullptr;auto* editor=new VST3Editor(this,"Main","tinyfreaky.uidesc");editor->setDelegate(this);return editor;}
}
