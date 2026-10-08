#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <proto/workbench.h>
#include <devices/inputevent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "module.h"
#include "popup.h"
#include "item_layout.h"
#include "server.h"
struct ViewItem {int builtin;uint32_t token;char label[32],text[100];};
struct Bar {struct Screen *screen;struct Window *window;struct DrawInfo *draw;struct SbItemLayout layout;struct ViewItem items[SB_MAX_ITEMS];int count;};
static struct Bar bars[2];
static struct SbServer server;
static struct SbPopup popup;
static struct ViewItem menu_items[SB_MAX_ITEMS];
static struct Bar *menu_bar;
static int menu_count,menu_action,quitting,title_right_override=-1;
static FILE *logfile;
static void record(const char *s) {if(logfile) {fprintf(logfile,"%s\n",s);fflush(logfile);}}
static void stroke(struct RastPort *rp,int x1,int y1,int x2,int y2) {Move(rp,x1,y1);Draw(rp,x2,y2);}
static void close_window(struct Bar *b) {
 if(!b->window)return;
 struct Message *m;while((m=GetMsg(b->window->UserPort)))ReplyMsg(m);
 CloseWindow(b->window);b->window=NULL;
}
static void paint(struct Bar *b) {
 if(!b->window)return;
 struct RastPort *rp=b->window->RPort;int h=b->window->Height,y=h/2,x=0;
 SetAPen(rp,b->draw->dri_Pens[BARBLOCKPEN]);RectFill(rp,0,0,b->window->Width-1,h-1);
 SetAPen(rp,b->draw->dri_Pens[BARDETAILPEN]);SetDrMd(rp,JAM1);
 for(int i=0;i<b->layout.count;i++) {
  struct ViewItem *item=&b->items[i];int text_x=x+8;
  if(item->builtin==SB_MODULE_SCREENS) {
   stroke(rp,x+9,y-5,x+20,y-5);stroke(rp,x+20,y-5,x+20,y+2);
   stroke(rp,x+20,y+2,x+9,y+2);stroke(rp,x+9,y+2,x+9,y-5);
   stroke(rp,x+6,y-2,x+6,y+5);stroke(rp,x+6,y+5,x+17,y+5);
  } else if(item->builtin==SB_MODULE_AUDIO) {
   RectFill(rp,x+8,y-2,x+11,y+2);stroke(rp,x+11,y-2,x+16,y-6);
   stroke(rp,x+16,y-6,x+16,y+6);stroke(rp,x+16,y+6,x+11,y+2);
   stroke(rp,x+20,y-3,x+22,y);stroke(rp,x+22,y,x+20,y+3);
  } else if(item->token) {
   stroke(rp,x+7,y-4,x+15,y-4);stroke(rp,x+15,y-4,x+15,y+4);
   stroke(rp,x+15,y+4,x+7,y+4);stroke(rp,x+7,y+4,x+7,y-4);text_x=x+23;
  }
  if(item->text[0]) {Move(rp,text_x,(h-rp->Font->tf_YSize)/2+rp->Font->tf_Baseline);Text(rp,(STRPTR)item->text,strlen(item->text));}
  x+=b->layout.widths[i];
 }
 if(b->layout.hidden) {
  stroke(rp,x+8,y-3,x+12,y);stroke(rp,x+12,y,x+8,y+3);
  stroke(rp,x+14,y-3,x+18,y);stroke(rp,x+18,y,x+14,y+3);
 }
}
static void refresh_bar(struct Bar *b,const char *clock) {
 struct ViewItem items[SB_MAX_ITEMS];memset(items,0,sizeof(items));int count=3;
 items[0].builtin=SB_MODULE_SCREENS;strcpy(items[0].label,"Screens");
 items[1].builtin=SB_MODULE_AUDIO;strcpy(items[1].label,"Audio settings");
 items[2].builtin=SB_MODULE_CLOCK;strcpy(items[2].label,"Clock");strcpy(items[2].text,clock);
 for(int i=0;i<SB_MAX_PROVIDERS;i++) {
  struct SbProviderItem *p=&server.registry.items[i];if(!p->token)continue;
  struct ViewItem *v=&items[count++];v->builtin=-1;v->token=p->token;
  strcpy(v->label,p->label);snprintf(v->text,sizeof(v->text),"%s%s%s",p->label,p->text[0]?": ":"",p->text);
 }
 struct RastPort measure;InitRastPort(&measure);SetFont(&measure,b->draw->dri_Font);
 int widths[SB_MAX_ITEMS];for(int i=0;i<count;i++)widths[i]=i<2?26:TextLength(&measure,(STRPTR)items[i].text,strlen(items[i].text))+(items[i].token?31:16);
 int title_width=0,reserve=36;
 ULONG lock=LockIBase(0);
 if(b->screen->Title)title_width=TextLength(&measure,b->screen->Title,strlen((char *)b->screen->Title));
 for(struct Gadget *g=b->screen->FirstGadget;g;g=g->NextGadget) {
  if(g->Flags&GFLG_RELRIGHT) {
   int left=b->screen->Width-1+g->LeftEdge;
   int need=b->screen->Width-left+4;if(need>reserve)reserve=need;
  }
 }
 UnlockIBase(lock);
 int title_right=(b->screen->Width+title_width)/2+12;
 if(title_right_override>=0)title_right=title_right_override;
 struct SbItemLayout layout;sb_items_layout(&layout,b->screen->Width,reserve,title_right,widths,count,26);
 int changed=count!=b->count || memcmp(items,b->items,sizeof(items)) || memcmp(&layout,&b->layout,sizeof(layout));
 struct SbItemTarget old_targets[SB_MAX_ITEMS],new_targets[SB_MAX_ITEMS];
 for(int i=0;i<b->count;i++) {old_targets[i].builtin=b->items[i].builtin;old_targets[i].token=b->items[i].token;}
 for(int i=0;i<count;i++) {new_targets[i].builtin=items[i].builtin;new_targets[i].token=items[i].token;}
 int geometry=memcmp(&layout,&b->layout,sizeof(layout))!=0;
 int replace=geometry || sb_targets_changed(old_targets,b->count,new_targets,count);
 /* Queued clicks belong to the old window's geometry and item identities. */
 if(replace)close_window(b);
 if(replace && popup.window && popup.screen==b->screen) {sb_popup_close(&popup);popup.result=-1;}
 b->count=count;memcpy(b->items,items,sizeof(items));b->layout=layout;
 if(!layout.width) {close_window(b);return;}
 if(!b->window) {
  b->window=OpenWindowTags(NULL,WA_CustomScreen,(IPTR)b->screen,WA_Left,layout.left,WA_Top,0,
   WA_Width,layout.width,WA_Height,b->screen->BarHeight+1,WA_Borderless,TRUE,WA_Activate,FALSE,
   WA_RMBTrap,TRUE,WA_IDCMP,IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW,TAG_DONE);
  if(!b->window) {record("overlay creation failed");return;}
  SetFont(b->window->RPort,b->draw->dri_Font);changed=1;
 }
 if(changed)paint(b);
 if(geometry) {char s[160];snprintf(s,sizeof(s),"layout: title-right=%d left=%d width=%d visible=%d hidden=%d reserve=%d",title_right,layout.left,layout.width,layout.count,layout.hidden,reserve);record(s);}
}
static struct SbProviderItem *provider(uint32_t token) {
 for(int i=0;i<SB_MAX_PROVIDERS;i++)if(server.registry.items[i].token==token)return &server.registry.items[i];
 return NULL;
}
static void activate(struct Bar *b,const struct ViewItem *item) {
 sb_popup_close(&popup);popup.result=-1;menu_count=0;
 if(item->token) {
  struct SbProviderItem *p=provider(item->token);if(!p || !p->action_label[0])return;
  int anchor=b->layout.left;
  for(int i=0;i<b->layout.count;i++) {
   if(b->items[i].token==item->token)break;
   anchor+=b->layout.widths[i];
  }
  if(anchor>=b->layout.left+b->layout.width)anchor=b->layout.left+b->layout.width-26;
  char labels[1][128];snprintf(labels[0],sizeof(labels[0]),"%s",p->action_label);
  menu_bar=b;menu_action=1;menu_count=1;menu_items[0]=*item;
  sb_popup_open_list(&popup,b->screen,b->draw,anchor,b->screen->BarHeight+2,labels,1);
 } else if(item->builtin==SB_MODULE_SCREENS)sb_popup_open(&popup,b->screen,b->draw,b->layout.left,b->screen->BarHeight+2);
 else if(item->builtin==SB_MODULE_AUDIO) {
  BOOL ok=OpenWorkbenchObject((STRPTR)"SYS:Prefs/AHI",TAG_DONE);record(ok?"AHI preferences opened":"AHI preferences could not be opened");
  if(!ok) {struct EasyStruct e={sizeof(e),0,(STRPTR)"ScreenBar",(STRPTR)"Could not open SYS:Prefs/AHI.",(STRPTR)"OK"};EasyRequestArgs(NULL,&e,NULL,NULL);}
 }
}
static void overflow(struct Bar *b) {
 sb_popup_close(&popup);popup.result=-1;menu_bar=b;menu_count=0;menu_action=0;
 char labels[SB_MAX_ITEMS][128];
 for(int i=b->layout.count;i<b->count;i++) {
  menu_items[menu_count]=b->items[i];
  snprintf(labels[menu_count],sizeof(labels[menu_count]),"%s",b->items[i].text[0]?b->items[i].text:b->items[i].label);menu_count++;
 }
 sb_popup_open_list(&popup,b->screen,b->draw,b->layout.left+b->layout.width-26,b->screen->BarHeight+2,labels,menu_count);
}
int main(int argc,char **argv) {
 int demo=0,seconds=0;
 for(int i=1;i<argc;i++) {
  if(!strcmp(argv[i],"--demo"))demo=1;
  else if(!strcmp(argv[i],"--seconds") && i+1<argc)seconds=atoi(argv[++i]);
  else if(!strcmp(argv[i],"--title-right") && i+1<argc)title_right_override=atoi(argv[++i]);
  else if(!strcmp(argv[i],"--log") && i+1<argc) {logfile=fopen(argv[++i],"w");if(!logfile)return 1;}
  else {fprintf(stderr,"Usage: ScreenBar [--demo] [--seconds N] [--title-right X] [--log PATH]\n");return 2;}
 }
 if(!sb_server_open(&server)) {record("server unavailable: another host or allocation failure");if(logfile)fclose(logfile);return 1;}
 struct Screen *wb=LockPubScreen(NULL),*ds=NULL;
 if(!wb) {sb_server_close(&server);if(logfile)fclose(logfile);return 1;}
 bars[0].screen=wb;bars[0].draw=GetScreenDrawInfo(wb);
 if(!bars[0].draw)quitting=1;
 if(demo && !quitting) {
  ds=OpenScreenTags(NULL,SA_Width,wb->Width,SA_Height,wb->Height,SA_Depth,GetBitMapAttr(wb->RastPort.BitMap,BMA_DEPTH),
   SA_DisplayID,GetVPModeID(&wb->ViewPort),SA_Title,(IPTR)"ScreenBar demo",SA_PubName,(IPTR)"ScreenBarDemo",SA_Behind,TRUE,TAG_DONE);
  if(ds) {PubScreenStatus(ds,0);bars[1].screen=ds;bars[1].draw=GetScreenDrawInfo(ds);record("demo screen created");}
 }
 popup.record=record;popup.result=-1;time_t start=time(NULL);
 record("started; application providers enabled");
 while(!quitting) {
  if(SetSignal(0,SIGBREAKF_CTRL_C)&SIGBREAKF_CTRL_C)break;
  if(seconds>0 && time(NULL)-start>=seconds)break;
  sb_server_pump(&server,sb_server_now(&server));
  if(popup.window && popup.kind)for(int i=0;i<menu_count;i++)if(menu_items[i].token && !provider(menu_items[i].token)) {sb_popup_close(&popup);popup.result=-1;break;}
  time_t now=time(NULL);struct tm *tm=localtime(&now);char clock[6];strftime(clock,sizeof(clock),"%H:%M",tm);
  for(int i=0;i<2;i++) {
   struct Bar *b=&bars[i];if(!b->draw)continue;refresh_bar(b,clock);if(!b->window)continue;
   struct IntuiMessage *msg;
   while((msg=(struct IntuiMessage *)GetMsg(b->window->UserPort))) {
    ULONG cls=msg->Class;UWORD code=msg->Code;int x=msg->MouseX;ReplyMsg((struct Message *)msg);
    if(cls==IDCMP_RAWKEY && code==0x45) {if(popup.window)sb_popup_close(&popup);else quitting=1;}
    else if(cls==IDCMP_REFRESHWINDOW) {BeginRefresh(b->window);paint(b);EndRefresh(b->window,TRUE);}
    else if(cls==IDCMP_MOUSEBUTTONS && code==SELECTDOWN) {
     if(popup.window) {sb_popup_close(&popup);popup.result=-1;continue;}
     int hit=sb_items_hit(&b->layout,x);
     if(hit==SB_HIT_OVERFLOW)overflow(b);
     else if(hit>=0)activate(b,&b->items[hit]);
    }
   }
  }
  sb_popup_pump(&popup);
  if(popup.result>=0) {
   int selected=popup.result;popup.result=-1;
   if(selected<menu_count) {
    struct ViewItem item=menu_items[selected];
    if(menu_action) {if(sb_registry_action(&server.registry,item.token))record("provider action queued");}
    else activate(menu_bar,&item);
   }
  }
  Delay(5);
 }
 sb_server_close(&server);sb_popup_close(&popup);
 for(int i=1;i>=0;i--) {close_window(&bars[i]);if(bars[i].draw)FreeScreenDrawInfo(bars[i].screen,bars[i].draw);}
 int failed=0;if(ds && !CloseScreen(ds)) {record("demo screen could not close: external visitors remain");failed=1;}
 UnlockPubScreen(NULL,wb);record(failed?"exit with demo screen still open":"clean exit");if(logfile)fclose(logfile);return failed;
}
