// SPDX-License-Identifier: MIT
#include "plugin.h"
#include "vstgui4/vstgui/lib/cbitmap.h"
#include "public.sdk/source/common/openurl.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "vstgui4/vstgui/lib/cdrawcontext.h"
#include "vstgui4/vstgui/lib/cfont.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
namespace Tinyfreaky {
using namespace VSTGUI;
static const CColor bg(16,43,37),surface(22,56,47),white(242,239,223),orange(241,138,60),green(184,219,145),dim(75,112,94),oled(7,15,21),cyan(217,248,255);
// Smooth chassis and inset gradients, matching the Force skin.
static void gradient(CDrawContext*d,const CRect&r,CColor top,CColor bottom){
 for(int y=(int)r.top;y<(int)r.bottom;++y){double t=(y-r.top)/(r.getHeight()-1);CColor c((uint8_t)(top.red+(bottom.red-top.red)*t),(uint8_t)(top.green+(bottom.green-top.green)*t),(uint8_t)(top.blue+(bottom.blue-top.blue)*t));d->setFillColor(c);d->drawRect(CRect(r.left,y,r.right,y+1),kDrawFilled);}
}
static int indexOf(const char*key){for(int i=0;i<Count;i++)if(!std::strcmp(key,definitions[i].key))return i;return -1;}
static const char* pageNames[]={"OSCILLATOR","FILTER + ENV","CYCLING ENV","LFO + PLAY","MATRIX","ASSIGN","RHYTHM","SETUP","SEQ"};
static const char* keys[9][8]={
 {"osc_type","osc_wave","osc_timbre","osc_shape","preset","master","glide","paraphonic"},
 {"filter_type","cutoff","resonance","filter_amount","attack","decay","sustain","amp_mod"},
 {"cyc_mode","cyc_rise","cyc_fall","cyc_hold","cyc_amount","cyc_rise_curve","cyc_fall_curve","tempo"},
 {"lfo_shape","lfo_rate_free","lfo_sync","lfo_rate_sync","hold","bend_range","octave","all_notes_off"},
 {},{"assign1","assign2","assign3"},{"arp_on","arp_mode","arp_rate","swing","seq_on","seq_rate","seq_length","tempo"},{"arp_octaves","arp_gate","rhythm_sync","seq_gate","seq_velocity","seq_transpose","seq_reset","hold"},{}};
static const char* labels[9][8]={
 {"ENGINE","WAVE","TIMBRE","SHAPE","PRESET","MASTER","GLIDE","PARAPHONIC"},
 {"MODE","CUTOFF","RESONANCE","ENV AMOUNT","ATTACK","DECAY / REL","SUSTAIN","AMP ENV"},
 {"MODE","RISE","FALL","HOLD","AMOUNT","RISE CURVE","FALL CURVE","TEMPO"},
 {"SHAPE","RATE","SYNC","DIVISION","HOLD","BEND RANGE","OCTAVE","PANIC"},
 {},{"ASSIGN 1","ASSIGN 2","ASSIGN 3"},{"ARP ON","ARP MODE","ARP RATE","SWING","SEQ PLAY","SEQ RATE","LENGTH","TEMPO"},{"ARP OCTAVES","ARP GATE","CLOCK","SEQ GATE","VELOCITY","TRANSPOSE","RESTART","HOLD"},{}};
struct MascotLine {int part;float x1,y1,x2,y2;};
static const MascotLine mascotLines[]={
#include "mascot.inc"
};
struct Cell {std::string key,label;double x,y,r;bool matrix;};
class Panel final:public CView {
 SharedPointer<CBitmap> wordmark;
 Controller*controller;int page=0,seqBank=0,editing=-1,focus=-1;double startY=0,startValue=0;std::vector<Cell>cells;
 std::array<double,Count> previous{};std::array<double,VisualCount> visualPrevious{};const mpc_engine_t*api=mpc_engine();void*display=nullptr;
 void text(CDrawContext*d,const char*s,double x,double y,double w,double h,int size=28,CColor colour=white){d->setFont(kNormalFont,size);d->setFontColor(colour);d->drawString(s,CRect(x,y,x+w,y+h),kCenterText);}
 std::string value(int id){String128 str{};controller->getParamStringByValue(id,controller->getParamNormalized(id),str);char b[192]{};UString(str,128).toAscii(b,sizeof b);auto&def=definitions[id];double raw=def.lo+controller->getParamNormalized(id)*(def.hi-def.lo);
  if(id==10||id==11){double t=.002*std::pow(10000.,id==11?std::sqrt(std::clamp(raw,0.,1.)):raw);std::snprintf(b,sizeof b,t<1?"%.0f ms":"%.2f s",t<1?t*1000:t);return b;}
  if(!std::strncmp(def.key,"seq_note_",9)){int pitch=(int)std::lround(raw);const char*notes[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};if(pitch<0)std::snprintf(b,sizeof b,"REST");else std::snprintf(b,sizeof b,"%s%d",notes[pitch%12],pitch/12-1);}
  else if(!std::strcmp(def.key,"cutoff"))std::snprintf(b,sizeof b,"%.0f Hz",30+19970*raw*raw*raw);
  else if(!std::strcmp(def.key,"filter_amount"))std::snprintf(b,sizeof b,"%+.0f",raw*200-100);
  else if(def.lo==0&&def.hi==1&&def.steps==0)std::snprintf(b,sizeof b,"%.0f%%",raw*100);
  else if(def.lo==-100&&def.hi==100)std::snprintf(b,sizeof b,"%+.0f",raw);
  return b;
 }
 void refreshCells(){cells.clear();if(page==4){const char*rows[]={"CYCLING ENV","ENVELOPE","LFO","PRESSURE","KEY / ARP"};for(int r=0;r<5;r++)for(int c=0;c<7;c++){char key[32];std::snprintf(key,sizeof key,"matrix_%d_%d",r,c);cells.push_back({key,rows[r],308.+145*c,316.+54*r,15,true});}}else if(page==8){for(int i=0;i<8;i++){int step=seqBank*8+i;cells.push_back({"seq_note_"+std::to_string(step),"STEP "+std::to_string(step+1),160.+320*(i%4),284.+180*(i/4),34,false});}}else for(int i=0;i<8&&keys[page][i];i++)cells.push_back({keys[page][i],labels[page][i],160.+320*(i%4),284.+180*(i/4),34,false});}
 void finish(){if(editing>=0){controller->endEdit(editing);editing=-1;}}
public:
 Panel(Controller*c):CView(CRect(0,0,1280,628)),controller(c){wordmark=VSTGUI::owned(new CBitmap(CResourceDescription("wordmark.png")));visualPrevious[35]=0;previous.fill(-1);display=api->create(nullptr);refreshCells();setWantsIdle(true);}
 ~Panel()override{finish();if(display)api->destroy(display);}
 void onIdle()override{bool changed=false;for(int i=0;i<Count;i++){double v=controller->getParamNormalized(i);if(v!=previous[i]){previous[i]=v;changed=true;}}for(int i=0;i<VisualCount;i++){double v=controller->getParamNormalized(VisualBase+i);if(v!=visualPrevious[i]){visualPrevious[i]=v;changed=true;}}if(changed)invalid();}
 void draw(CDrawContext*d)override{
  d->setDrawMode(kAntiAliasing);gradient(d,getViewSize(),CColor(24,58,48),CColor(14,37,31));if(wordmark)wordmark->draw(d,CRect(24,2,324,54));text(d,pageNames[page],450,13,380,34,24,green);text(d,"MagicStino  /  GitHub",908,10,340,24,21,orange);text(d,"0.3.8",1140,35,100,18,13,green);
  d->setFillColor(oled);d->drawRect(CRect(24,60,1256,234),kDrawFilled);gradient(d,CRect(24,252,1256,574),CColor(28,64,53),CColor(19,48,39));d->setFrameColor(CColor(80,114,96));d->setLineWidth(1);d->drawLine(CPoint(24,252),CPoint(1256,252));
  auto preset=value(0);if(preset.rfind("~ ",0)==0)preset.erase(0,2);text(d,preset.c_str(),48,68,800,54,36,cyan);
  int model=indexOf("osc_type");std::string modelText="ENGINE  "+value(model);text(d,modelText.c_str(),48,125,800,48,36,white);
  int routes=0;for(int i=0;i<Count;i++)if(!std::strncmp(definitions[i].key,"matrix_",7)&&std::abs(controller->getParamNormalized(i)-.5)>.00005)routes++;
  char routeText[48];std::snprintf(routeText,sizeof routeText,"%d ROUTES",routes);text(d,routeText,900,184,320,38,24,orange);
  std::string detail=focus>=0?std::string(definitions[focus].name)+"   "+value(focus):"TOUCH A CONTROL  /  DRAG TO ADJUST";text(d,detail.c_str(),48,180,800,44,26,cyan);
  text(d,"E L",846,101,36,18,12,green);
  text(d,"OUTPUT SCOPE / AUTO",890,70,320,28,18,green);
  d->setFrameColor(CColor(36,60,70));d->setLineWidth(1);d->drawLine(CPoint(890,132),CPoint(1210,132));d->setFrameColor(cyan);d->setLineWidth(2);
  CPoint last(890,132);for(int i=0;i<32;i++){CPoint next(895+i*10,156-48*visualPrevious[i]);if(i)d->drawLine(last,next);last=next;}
  d->setFillColor(green);d->drawRect(CRect(850,160-56*visualPrevious[32],856,160),kDrawFilled);
  int range=(int)std::lround(visualPrevious[33]*152),hi=0;while(hi<16&&(hi+1)*(hi+2)/2<=range)hi++;int lo=range-hi*(hi+1)/2;
  double top=156-hi*3.,bottom=156-lo*3.;d->setFillColor(CColor(146,86,44));d->drawRect(CRect(868,top,874,std::max(top+2,bottom)),kDrawFilled);d->setFrameColor(orange);d->setLineWidth(2);d->drawLine(CPoint(866,top),CPoint(876,top));d->drawLine(CPoint(866,bottom),CPoint(876,bottom));
  int state=(int)std::lround(visualPrevious[34]*3);d->setFillColor(state==1?orange:state==2?green:dim);d->setFrameColor(state==1?orange:state==2?green:dim);
  if(state==1){d->setLineWidth(3);d->drawLine(CPoint(1220,82),CPoint(1238,92));d->drawLine(CPoint(1238,92),CPoint(1220,102));d->drawLine(CPoint(1220,102),CPoint(1220,82));}else if(state==2){d->drawRect(CRect(1220,82,1226,102),kDrawFilled);d->drawRect(CRect(1232,82,1238,102),kDrawFilled);}else d->drawRect(CRect(1220,84,1238,102),state==3?kDrawStroked:kDrawFilled);
  if(display){char raw[24];std::snprintf(raw,sizeof raw,"%.0f",controller->getParamNormalized(model)*15);api->set_param(display,"osc_type",raw);}
  if(page==4){const char*rows[]={"CYCLING ENV","ENVELOPE","LFO","PRESSURE","KEY / ARP"};const char*cols[]={"PITCH","WAVE","TIMBRE","CUTOFF","ASSIGN 1","ASSIGN 2","ASSIGN 3"};for(int c=0;c<7;c++)text(d,cols[c],240+c*145,256,136,34,22);for(int r=0;r<5;r++)text(d,rows[r],32,296+r*54,194,36,22,green);}
  for(auto&cell:cells){int id=indexOf(cell.key.c_str());double norm=controller->getParamNormalized(id);bool quiet=cell.matrix&&std::abs(norm-.5)<.00005;CColor colour=quiet?dim:orange;if(!cell.matrix){colour=green;if(id==3||id==0)colour=white;else if(id>=4&&id<=6||cell.key=="arp_on"||cell.key=="seq_on"||cell.key=="arp_mode")colour=orange;else if(definitions[id].steps>0)colour=oled;}
   d->setFillColor(colour);d->drawEllipse(CRect(cell.x-cell.r,cell.y-cell.r,cell.x+cell.r,cell.y+cell.r),kDrawFilled);double angle=(135+270*norm)*3.141592653589793/180.;d->setFrameColor(colour==oled?white:bg);d->setLineWidth(3);d->drawLine(CPoint(cell.x,cell.y),CPoint(cell.x+std::cos(angle)*(cell.r-5),cell.y+std::sin(angle)*(cell.r-5)));
   auto val=value(id);if(cell.matrix)text(d,val.c_str(),cell.x+20,cell.y-22,96,44,28,quiet?green:white);
   else{std::string label=cell.label;if(display&&id>=4&&id<=6){char b[128],key[32];std::snprintf(key,sizeof key,"%s_name",cell.key.c_str());if(api->get_param(display,key,b,sizeof b)>0)label=b;}text(d,label.c_str(),cell.x-(cell.y>400?126:148),cell.y+40,cell.y>400?252:296,38,26);text(d,val.c_str(),cell.x-(cell.y>400?126:148),cell.y+79,cell.y>400?252:296,38,28,green);}
   if(editing==id){d->setFrameColor(white);d->setLineWidth(2);d->drawRect(CRect(cell.x-cell.r-5,cell.y-cell.r-5,cell.x+cell.r+5,cell.y+cell.r+5));}
  }
  if(page!=4){int pose=std::clamp((int)std::lround(visualPrevious[35]*540),0,540);int style=pose?(pose-1)/60:0;double t=pose?((pose-1)%60)*6.28318530718/60:0,high=(style/3)*.5,v=pose?(style%3==0?.22:style%3==1?.55:1):0,hip=3*(1.6*v+8.6*(1-high)*std::pow(v,.4))*std::sin(t),torso=(2+8*high)*v*std::sin(t+.55)*3.14159265359/180,head=(4+17*v*v+5*high)*v*std::sin(t+1.1)*3.14159265359/180,bob=(.8+4.2*v*v)*v*std::sin(2*t+.7);
   auto point=[&](double x,double y,int part){if(part==0){double dx=x-49,dy=y-73;x=49+dx*std::cos(head)-dy*std::sin(head);y=73+dx*std::sin(head)+dy*std::cos(head)+bob;}if(part==2)x+=hip*(1-std::clamp((y-150)/64,0.,1.));else{double dx=x-49,dy=y-150;x=49+hip+dx*std::cos(torso)-dy*std::sin(torso);y=150+dx*std::sin(torso)+dy*std::cos(torso);}return CPoint(605+x*.7,404+y*.72);};
   d->setFrameColor(white);d->setLineWidth(1.15);for(const auto&line:mascotLines)d->drawLine(point(line.x1,line.y1,line.part),point(line.x2,line.y2,line.part));}
  if(page==8){text(d,"< PREVIOUS 8",30,184,250,40,22,orange);text(d,"NEXT 8 >",1000,184,250,40,22,orange);}
  for(int i=0;i<9;i++){d->setFillColor(i==page?orange:surface);d->drawRect(CRect(24+i*138,590,152+i*138,622),kDrawFilled);text(d,pageNames[i],24+i*138,591,128,30,17,i==page?bg:white);}setDirty(false);
 }
 CMouseEventResult onMouseDown(CPoint&p,const CButtonState&b)override{if(!b.isLeftButton())return kMouseEventNotHandled;finish();if(p.x>=908&&p.x<=1248&&p.y<=54){Steinberg::openURLInDefaultApplication("https://github.com/MagicStino/TinyFreaky");return kMouseDownEventHandledButDontNeedMovedOrUpEvents;}if(page==8&&p.y>=180&&p.y<=228&&(p.x<280||p.x>1000)){seqBank=std::clamp(seqBank+(p.x<280?-1:1),0,7);refreshCells();invalid();return kMouseDownEventHandledButDontNeedMovedOrUpEvents;}if(p.y>=590&&p.x>=24&&p.x<1266){page=std::min(8,int((p.x-24)/138));refreshCells();invalid();return kMouseDownEventHandledButDontNeedMovedOrUpEvents;}for(auto&cell:cells){bool hit=cell.matrix?p.x>=cell.x-24&&p.x<=cell.x+118&&std::abs(p.y-cell.y)<=25:std::abs(p.x-cell.x)<=146&&p.y>=cell.y-cell.r-7&&p.y<=cell.y+118;if(hit){editing=focus=indexOf(cell.key.c_str());startY=p.y;startValue=controller->getParamNormalized(editing);controller->beginEdit(editing);if(!std::strcmp(cell.key.c_str(),"all_notes_off")||!std::strcmp(cell.key.c_str(),"seq_reset")){controller->setParamNormalized(editing,1);controller->performEdit(editing,1);finish();}invalid();return kMouseEventHandled;}}return kMouseEventNotHandled;}
 CMouseEventResult onMouseMoved(CPoint&p,const CButtonState&)override{if(editing<0)return kMouseEventNotHandled;double v=std::clamp(startValue+(startY-p.y)/240.,0.,1.);auto&def=definitions[editing];if(def.steps)v=std::round(v*def.steps)/def.steps;controller->setParamNormalized(editing,v);controller->performEdit(editing,v);invalid();return kMouseEventHandled;}
 CMouseEventResult onMouseUp(CPoint&,const CButtonState&)override{finish();invalid();return kMouseEventHandled;}
 CMouseEventResult onMouseCancel()override{finish();invalid();return kMouseEventHandled;}
};
CView*Controller::createCustomView(UTF8StringPtr name,const UIAttributes&,const IUIDescription*,VST3Editor*){return !std::strcmp(name,"TinyfreakyPanel")?new Panel(this):nullptr;}
IPlugView*PLUGIN_API Controller::createView(FIDString name){if(std::strcmp(name,ViewType::kEditor))return nullptr;auto*editor=new VST3Editor(this,"Main","tinyfreaky.uidesc");editor->setDelegate(this);return editor;}
}
