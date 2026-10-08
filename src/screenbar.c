/* ScreenBar POC: native borderless overlays, never writes system decoration. */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>
#include <graphics/gfxbase.h>
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
#include "layout.h"
#include "module.h"
#include "popup.h"
struct Bar { struct Screen *screen; struct Window *window; struct DrawInfo *draw; int cell; char time[32]; };
static struct Bar bars[2];
static struct SbModuleState modules[3];
static struct SbPopup popup;
static int quitting;
static FILE *logfile;
static void record(const char *s) { if(logfile) { fprintf(logfile,"%s\n",s); fflush(logfile); } }
static void text(struct RastPort *rp,int x,int y,const char *s) { Move(rp,x,y); Text(rp,(STRPTR)s,strlen(s)); }
static void stroke(struct RastPort *rp,int x1,int y1,int x2,int y2) { Move(rp,x1,y1); Draw(rp,x2,y2); }
static void paint(struct Bar *b) {
    struct RastPort *rp=b->window->RPort;
    int h=b->window->Height, c=b->cell, y=h/2;
    SetAPen(rp,b->draw->dri_Pens[BARBLOCKPEN]); RectFill(rp,0,0,b->window->Width-1,h-1);
    SetAPen(rp,b->draw->dri_Pens[BARDETAILPEN]); SetDrMd(rp,JAM1);
    /* Host rendering consumes module state; providers own no drawing objects. */
    for(int i=0;i<3;i++) {
        int x=i*c;
        if(modules[i].icon==SB_ICON_SCREENS) {
            stroke(rp,x+9,y-5,x+20,y-5);stroke(rp,x+20,y-5,x+20,y+2);
            stroke(rp,x+20,y+2,x+9,y+2);stroke(rp,x+9,y+2,x+9,y-5);
            stroke(rp,x+6,y-2,x+6,y+5);stroke(rp,x+6,y+5,x+17,y+5);
        } else if(modules[i].icon==SB_ICON_AUDIO) {
            RectFill(rp,x+8,y-2,x+11,y+2);
            stroke(rp,x+11,y-2,x+16,y-6);stroke(rp,x+16,y-6,x+16,y+6);
            stroke(rp,x+16,y+6,x+11,y+2);stroke(rp,x+20,y-3,x+22,y);stroke(rp,x+22,y,x+20,y+3);
        }
        if(modules[i].text[0]) {
            int baseline=(h-rp->Font->tf_YSize)/2+rp->Font->tf_Baseline;
            text(rp,x+9,baseline,modules[i].text);
        }
    }
}
static int create_bar(struct Bar *b,struct Screen *s) {
    memset(b,0,sizeof(*b));b->screen=s;b->draw=GetScreenDrawInfo(s);
    if(!b->draw) return 0;
    int h=s->BarHeight+1;b->cell=h<26?26:h;
    int width=2*b->cell+5*b->draw->dri_Font->tf_XSize+20;
    if(width+36>s->Width) {FreeScreenDrawInfo(s,b->draw);b->draw=NULL;record("bar unavailable: insufficient screen width");return 0;}
    b->window=OpenWindowTags(NULL,WA_CustomScreen,(IPTR)s,WA_Left,sb_left(s->Width,width,36),WA_Top,0,
        WA_Width,width,WA_Height,h,WA_Borderless,TRUE,WA_Activate,FALSE,
        WA_RMBTrap,TRUE,WA_IDCMP,IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW,TAG_DONE);
    if(!b->window) { FreeScreenDrawInfo(s,b->draw);b->draw=NULL;return 0; }
    SetFont(b->window->RPort,b->draw->dri_Font); strcpy(b->time,"--:--");paint(b);
    char report[160];snprintf(report,sizeof(report),"bar: %dx%d at %d,%d; screen=%s",b->window->Width,b->window->Height,b->window->LeftEdge,b->window->TopEdge,s->Title?(char*)s->Title:"untitled");record(report);
    return 1;
}
int main(int argc,char **argv) {
    int demo=0,seconds=0;
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--demo"))demo=1;
        else if(!strcmp(argv[i],"--seconds") && i+1<argc)seconds=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--log") && i+1<argc)logfile=fopen(argv[++i],"w");
        else { fprintf(stderr,"Usage: ScreenBar [--demo] [--seconds N] [--log PATH]\n");return 2; }
    }
    sb_modules_update(modules,"--:--");
    struct Screen *wb=LockPubScreen(NULL),*ds=NULL;
    if(!wb || !create_bar(&bars[0],wb)) { record("failed to create Workbench bar");if(wb)UnlockPubScreen(NULL,wb);return 1; }
    if(demo) {
        ds=OpenScreenTags(NULL,SA_Width,wb->Width,SA_Height,wb->Height,SA_Depth,GetBitMapAttr(wb->RastPort.BitMap,BMA_DEPTH),
            SA_DisplayID,GetVPModeID(&wb->ViewPort),SA_Title,(IPTR)"ScreenBar demo",SA_PubName,(IPTR)"ScreenBarDemo",SA_Behind,TRUE,TAG_DONE);
        if(ds) { PubScreenStatus(ds,0); create_bar(&bars[1],ds);record("demo screen created"); }
    }
    popup.record=record;
    time_t start=time(NULL);
    record("started; volume icon opens AHI preferences; Escape or Ctrl-C quits");
    while(!quitting) {
        if(SetSignal(0,SIGBREAKF_CTRL_C)&SIGBREAKF_CTRL_C)break;
        if(seconds>0 && time(NULL)-start>=seconds)break;
        time_t now=time(NULL); struct tm *tm=localtime(&now);char clock[6];strftime(clock,sizeof(clock),"%H:%M",tm);
        sb_modules_update(modules,clock);
        for(int i=0;i<2;i++) {
            struct Bar *b=&bars[i];if(!b->window)continue;
            if(strcmp(clock,b->time)) {strcpy(b->time,clock);paint(b);}
            struct IntuiMessage *msg;
            while((msg=(struct IntuiMessage*)GetMsg(b->window->UserPort))) {
                ULONG cls=msg->Class;UWORD code=msg->Code;int x=msg->MouseX;ReplyMsg((struct Message*)msg);
                if(cls==IDCMP_RAWKEY && code==0x45)quitting=1;
                else if(cls==IDCMP_REFRESHWINDOW) { BeginRefresh(b->window);paint(b);EndRefresh(b->window,TRUE); }
                else if(cls==IDCMP_MOUSEBUTTONS && code==SELECTDOWN) {
                    int hit=sb_cell_hit(x,b->window->Width,b->cell);
                    enum SbAction action=hit<0?SB_ACTION_NONE:sb_module_action(modules[hit].id);
                    if(action==SB_ACTION_SHOW_SCREENS) {
                        if(popup.window)sb_popup_close(&popup);
                        else sb_popup_open(&popup,b->screen,b->draw,b->window->LeftEdge,b->screen->BarHeight+2);
                    } else if(action==SB_ACTION_OPEN_AHI) {
                        BOOL ok=OpenWorkbenchObject((STRPTR)"SYS:Prefs/AHI",TAG_DONE);
                        record(ok?"AHI preferences opened":"AHI preferences could not be opened");
                        if(!ok) {struct EasyStruct e={sizeof(e),0,(STRPTR)"ScreenBar",(STRPTR)"Could not open SYS:Prefs/AHI.",(STRPTR)"OK"};EasyRequestArgs(NULL,&e,NULL,NULL);}
                    } else sb_popup_close(&popup);
                }
            }
        }
        sb_popup_pump(&popup);
        Delay(5);
    }
    sb_popup_close(&popup);
    for(int i=1;i>=0;i--)if(bars[i].window) {CloseWindow(bars[i].window);FreeScreenDrawInfo(bars[i].screen,bars[i].draw);}
    int failed=0;
    if(ds && !CloseScreen(ds)) {record("demo screen could not close: external visitors remain");failed=1;}
    UnlockPubScreen(NULL,wb);record(failed?"exit with demo screen still open":"clean exit");if(logfile)fclose(logfile);return failed;
}
