#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <intuition/cghooks.h>
#include <graphics/rastport.h>
#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct ClockData
{
    struct DrawInfo *draw;
    char text[6];
};

static IPTR clock_dispatcher(Class *cl, Object *obj, Msg msg)
{
    if (msg->MethodID == GM_RENDER)
    {
        struct ClockData *data = INST_DATA(cl, obj);
        struct gpRender *render = (struct gpRender *)msg;
        struct GadgetInfo *gi = render->gpr_GInfo;
        struct Gadget *gadget = (struct Gadget *)obj;
        struct RastPort *rp = render->gpr_RPort;
        LONG left = gi->gi_Domain.Width + gadget->LeftEdge;
        LONG textWidth = TextLength(rp, (STRPTR)data->text, strlen(data->text));
        LONG baseline = gadget->TopEdge +
            ((gadget->Height - rp->Font->tf_YSize) / 2) + rp->Font->tf_Baseline;

        SetAPen(rp, data->draw->dri_Pens[BARDETAILPEN]);
        SetDrMd(rp, JAM1);
        Move(rp, left + gadget->Width - textWidth - 6, baseline);
        Text(rp, (STRPTR)data->text, strlen(data->text));
        return 1;
    }
    return DoSuperMethodA(cl, obj, msg);
}

int main(int argc, char **argv)
{
    struct Screen *screen;
    struct DrawInfo *draw;
    struct IClass *cl;
    Object *clock;
    struct Gadget *gadget;
    struct TagItem tags[] = {
        {GA_Width, 76}, {GA_Height, 0}, {GA_Top, 0},
        {TAG_DONE, 0}
    };
    LONG seconds = argc > 1 ? atol(argv[1]) : 60;
    time_t start = time(NULL);
    BOOL attached = FALSE;

    /* The exported calls only exist in the matching ScreenBar Intuition build. */
    if (!IntuitionBase || ((struct Library *)IntuitionBase)->lib_Version != 50 ||
        ((struct Library *)IntuitionBase)->lib_Revision < 11)
        return RETURN_FAIL;

    screen = LockPubScreen(NULL);
    if (!screen) return RETURN_FAIL;
    draw = GetScreenDrawInfo(screen);
    if (!draw) { UnlockPubScreen(NULL, screen); return RETURN_FAIL; }

    cl = MakeClass(NULL, (ClassID)"gadgetclass", NULL, sizeof(struct ClockData), 0);
    if (!cl) goto fail_draw;
    cl->cl_Dispatcher.h_Entry = HookEntry;
    cl->cl_Dispatcher.h_SubEntry = (HOOKFUNC)clock_dispatcher;

    tags[1].ti_Data = screen->BarHeight - 2;
    tags[2].ti_Data = 1;
    clock = NewObjectA(cl, NULL, tags);
    if (!clock) goto fail_class;
    gadget = (struct Gadget *)clock;
    gadget->Flags |= GFLG_RELRIGHT;
    gadget->LeftEdge = -102; /* 76 px clock, 22 px depth gadget, 4 px gap. */
    gadget->GadgetType |= GTYP_SCRGADGET;
    gadget->Height = screen->BarHeight - 2;
    ((struct ClockData *)INST_DATA(cl, clock))->draw = draw;

    if (!AddScreenBarGadget(screen, gadget, 0)) goto fail_object;
    attached = TRUE;
    while (seconds <= 0 || time(NULL) - start < seconds)
    {
        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        strftime(((struct ClockData *)INST_DATA(cl, clock))->text,
                 sizeof(((struct ClockData *)INST_DATA(cl, clock))->text),
                 "%H:%M", tm);
        if (!RefreshScreenBarGadget(screen, gadget)) break;
        Delay(25);
    }

    if (attached) RemoveScreenBarGadget(screen, gadget);
    DisposeObject(clock);
    FreeClass(cl);
    FreeScreenDrawInfo(screen, draw);
    UnlockPubScreen(NULL, screen);
    return RETURN_OK;

fail_object:
    DisposeObject(clock);
fail_class:
    FreeClass(cl);
fail_draw:
    FreeScreenDrawInfo(screen, draw);
    UnlockPubScreen(NULL, screen);
    return RETURN_FAIL;
}
