#include "popup.h"
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <stdio.h>
#include <string.h>
#define PAD 4
static void report(struct SbPopup *p,const char *s) {if(p->record)p->record(s);}
static void paint(struct SbPopup *p) {
 struct Window *w=p->window; struct RastPort *rp=w->RPort;
 SetAPen(rp,p->draw->dri_Pens[BARBLOCKPEN]);RectFill(rp,0,0,w->Width-1,w->Height-1);
 SetAPen(rp,p->draw->dri_Pens[BARDETAILPEN]);
 Move(rp,0,0);Draw(rp,w->Width-1,0);Draw(rp,w->Width-1,w->Height-1);Draw(rp,0,w->Height-1);Draw(rp,0,0);
 for(int row=0;row<p->state.visible;row++) {
  int index=p->state.first+row, top=PAD+row*p->row_height;
  int selected=index==p->state.selected;
  SetAPen(rp,p->draw->dri_Pens[selected?FILLPEN:BARBLOCKPEN]);RectFill(rp,2,top,w->Width-3,top+p->row_height-1);
  SetAPen(rp,p->draw->dri_Pens[selected?FILLTEXTPEN:BARDETAILPEN]);SetDrMd(rp,JAM1);
  char label[140];snprintf(label,sizeof(label),"%s %s",index==p->current?"*":" ",p->names[index]);
  int maxchars=(w->Width-20)/rp->Font->tf_XSize;
  if(maxchars<0)maxchars=0;
  int len=strlen(label);if(len>maxchars)len=maxchars;
  Move(rp,8,top+(p->row_height-rp->Font->tf_YSize)/2+rp->Font->tf_Baseline);Text(rp,(STRPTR)label,len);
 }
 SetAPen(rp,p->draw->dri_Pens[BARDETAILPEN]);
 if(p->state.first>0) {Move(rp,w->Width-8,2);Draw(rp,w->Width-5,1);Draw(rp,w->Width-2,2);}
 if(p->state.first+p->state.visible<p->state.count) {Move(rp,w->Width-8,w->Height-3);Draw(rp,w->Width-5,w->Height-2);Draw(rp,w->Width-2,w->Height-3);}
}
void sb_popup_close(struct SbPopup *p) {
 if(p->window) {struct IntuiMessage *m;while((m=(struct IntuiMessage*)GetMsg(p->window->UserPort)))ReplyMsg((struct Message*)m);
 CloseWindow(p->window);p->window=NULL;report(p,"dropdown closed");}
}
static int open_window(struct SbPopup *p,struct Screen *s,struct DrawInfo *draw,int x,int y) {
 int width=120;
 for(int i=0;i<p->state.count;i++) {
  int need=(strlen(p->names[i])+2)*draw->dri_Font->tf_XSize+20;
  if(need>width)width=need;
 }
 if(!p->state.count || s->Width<32 || s->Height-y<28) {report(p,"dropdown unavailable: no screens or insufficient space");return 0;}
 p->row_height=draw->dri_Font->tf_YSize+8;if(p->row_height<20)p->row_height=20;
 int visible=(s->Height-y-2*PAD)/p->row_height;if(visible>p->state.count)visible=p->state.count;
 if(visible<1)return 0;
 sb_dropdown_init(&p->state,p->state.count,visible);
 if(p->current>=0)sb_dropdown_move(&p->state,p->current);
 if(width>s->Width)width=s->Width;
 if(x+width>s->Width)x=s->Width-width;
 if(x<0)x=0;
 p->window=OpenWindowTags(NULL,WA_CustomScreen,(IPTR)s,WA_Left,x,WA_Top,y,
  WA_Width,width,WA_Height,2*PAD+visible*p->row_height,WA_Borderless,TRUE,WA_Activate,TRUE,
  WA_RMBTrap,TRUE,WA_ReportMouse,TRUE,WA_IDCMP,IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW|IDCMP_INACTIVEWINDOW,TAG_DONE);
 if(!p->window) {report(p,"dropdown window creation failed");return 0;}
 SetFont(p->window->RPort,draw->dri_Font);paint(p);report(p,"dropdown opened");return 1;
}
int sb_popup_open(struct SbPopup *p,struct Screen *s,struct DrawInfo *draw,int x,int y) {
 sb_popup_close(p);p->screen=s;p->draw=draw;p->current=-1;p->pressed=-1;
 int count=0;p->kind=0;p->result=-1;
 struct List *list=LockPubScreenList();
 if(list) {
  for(struct PubScreenNode *n=(struct PubScreenNode*)list->lh_Head;n->psn_Node.ln_Succ && count<SB_MAX_SCREENS;n=(struct PubScreenNode*)n->psn_Node.ln_Succ) {
   if(!n->psn_Node.ln_Name || strlen(n->psn_Node.ln_Name)>=sizeof(p->names[0]))continue;
   strcpy(p->names[count],n->psn_Node.ln_Name);
   if(n->psn_Screen==s)p->current=count;
   count++;
  }
  UnlockPubScreenList();
 }
 p->state.count=count;return open_window(p,s,draw,x,y);
}
int sb_popup_open_list(struct SbPopup *p,struct Screen *s,struct DrawInfo *draw,int x,int y,const char labels[][128],int count) {
 sb_popup_close(p);p->screen=s;p->draw=draw;p->current=-1;p->pressed=-1;p->kind=1;p->result=-1;
 if(count<1 || count>SB_MAX_SCREENS)return 0;
 for(int i=0;i<count;i++) {
  if(strlen(labels[i])>=sizeof(p->names[i]))return 0;
  strcpy(p->names[i],labels[i]);
 }
 p->state.count=count;return open_window(p,s,draw,x,y);
}
static void choose(struct SbPopup *p,int index) {
 if(index<0 || index>=p->state.count)return;
 if(p->kind) {p->result=index;sb_popup_close(p);return;}
 struct Screen *s=LockPubScreen((STRPTR)p->names[index]);
 if(s) {ScreenToFront(s);UnlockPubScreen((STRPTR)p->names[index],s);report(p,p->names[index]);sb_popup_close(p);}
 else {
  report(p,"selected screen is no longer available");
  struct EasyStruct e={sizeof(e),0,(STRPTR)"ScreenBar",(STRPTR)"The selected screen is no longer available.",(STRPTR)"OK"};
  sb_popup_close(p);EasyRequestArgs(NULL,&e,NULL,NULL);
 }
}
void sb_popup_pump(struct SbPopup *p) {
 if(!p->window)return;
 ULONG lock=LockIBase(0);int front=IntuitionBase->FirstScreen==p->screen;UnlockIBase(lock);
 if(!front) {sb_popup_close(p);return;}
 struct IntuiMessage *m;
 while(p->window && (m=(struct IntuiMessage*)GetMsg(p->window->UserPort))) {
  ULONG cls=m->Class;UWORD code=m->Code;
  int hit=sb_dropdown_hit(&p->state,m->MouseX,m->MouseY,p->window->Width,p->row_height,PAD);
  ReplyMsg((struct Message*)m);
  if(cls==IDCMP_INACTIVEWINDOW) {report(p,"dropdown dismissed on focus loss; outside click is not consumed");sb_popup_close(p);}
  else if(cls==IDCMP_REFRESHWINDOW) {BeginRefresh(p->window);paint(p);EndRefresh(p->window,TRUE);report(p,"dropdown refreshed");}
  else if(cls==IDCMP_MOUSEMOVE && hit>=0) {sb_dropdown_move(&p->state,hit-p->state.selected);paint(p);}
  else if(cls==IDCMP_MOUSEBUTTONS) {
   if(code==SELECTDOWN)p->pressed=hit;
   else if(code==SELECTUP) {int pressed=p->pressed;p->pressed=-1;if(hit>=0 && hit==pressed)choose(p,hit);}
   else if(code==MENUDOWN)sb_popup_close(p);
  } else if(cls==IDCMP_RAWKEY) {
   enum SbDropdownKey key=sb_dropdown_key(code);
   if(key==SB_DD_DISMISS)sb_popup_close(p);
   else if(key==SB_DD_ACCEPT)choose(p,sb_dropdown_commit(&p->state));
   else if(key==SB_DD_UP || key==SB_DD_DOWN) {sb_dropdown_move(&p->state,key==SB_DD_UP?-1:1);paint(p);report(p,"dropdown keyboard selection");}
  }
 }
}
