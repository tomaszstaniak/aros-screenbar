/* Host-owned native rendering POC for the patched ABIv11 Intuition. */
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
#include <proto/cybergraphics.h>
#include <proto/dos.h>
#include <stdlib.h>
#include <string.h>
#include "../src/widget_visual.h"

#define ICON_SIZE 16
#define WIDGET_COUNT 4
#define WIDGET_MONO 0
#define WIDGET_COLOR 1
#define WIDGET_ANIMATED 2
#define WIDGET_WIDE 3

struct GalleryData
{
    struct DrawInfo *draw;
    ULONG mono_rgb;
    ULONG pixels[ICON_SIZE * ICON_SIZE];
    int widths[WIDGET_COUNT];
    unsigned frame;
};

static const unsigned short anim_durations[] = {180, 240, 320};
static const int preferred_widths[WIDGET_COUNT] = {30, 30, 30, 108};
static const int intrinsic_widths[WIDGET_COUNT] = {26, 26, 26, 88};
static const char *widget_labels[WIDGET_COUNT] = {"", "", "", "Copy 63%"};

static BOOL icon_pixel(unsigned kind, unsigned frame, unsigned x, unsigned y)
{
    if (kind == WIDGET_MONO)
        return (x == y && x >= 3 && x <= 12) ||
               (x + y == 15 && x >= 3 && x <= 12) ||
               (x == 7 && y >= 4 && y <= 11) ||
               (y == 7 && x >= 4 && x <= 11);

    if (kind == WIDGET_COLOR)
    {
        unsigned dx = x > 7 ? x - 7 : 7 - x;
        unsigned dy = y > 7 ? y - 7 : 7 - y;
        return dx * dx + dy * dy <= 45;
    }

    if (kind == WIDGET_ANIMATED)
    {
        unsigned column = x / 4;
        unsigned height = sb_animation_bar_height(frame, column);
        return y >= ICON_SIZE - height * 4;
    }

    return FALSE;
}

static void make_pixels(struct GalleryData *data, unsigned kind)
{
    unsigned x, y;
    for (y = 0; y < ICON_SIZE; ++y)
        for (x = 0; x < ICON_SIZE; ++x)
        {
            ULONG pixel = 0;
            if (icon_pixel(kind, data->frame, x, y))
            {
                if (kind == WIDGET_MONO)
                    pixel = data->mono_rgb;
                else if (kind == WIDGET_COLOR)
                    pixel = x < 8 ? 0xFF27A8F5UL : 0xFFFFA52BUL;
                else
                {
                    static const ULONG frame_colors[] = {
                        0xFF58D68DUL, 0xFFF4D03FUL, 0xFFEC7063UL
                    };
                    pixel = frame_colors[data->frame % 3];
                }
            }
            data->pixels[y * ICON_SIZE + x] = pixel;
        }
}

static IPTR gallery_dispatcher(Class *cl, Object *obj, Msg msg)
{
    if (msg->MethodID == GM_RENDER)
    {
        struct GalleryData *data = INST_DATA(cl, obj);
        struct gpRender *render = (struct gpRender *)msg;
        struct GadgetInfo *gi = render->gpr_GInfo;
        struct Gadget *gadget = (struct Gadget *)obj;
        struct RastPort *rp = render->gpr_RPort;
        LONG left = gi->gi_Domain.Width + gadget->LeftEdge;
        LONG top = gadget->TopEdge + (gadget->Height - ICON_SIZE) / 2;
        LONG text_y = gadget->TopEdge +
            ((gadget->Height - rp->Font->tf_YSize) / 2) + rp->Font->tf_Baseline;
        int i, x = left;

        for (i = 0; i < WIDGET_COUNT; ++i)
        {
            make_pixels(data, i);
            WritePixelArrayAlpha(data->pixels, 0, 0, ICON_SIZE * sizeof(ULONG), rp,
                                 x + 5, top, ICON_SIZE, ICON_SIZE, 0xFFFFFFFFUL);

            if (i == WIDGET_WIDE)
            {
                LONG bar_left = x + 29;
                LONG bar_top = top + ICON_SIZE - 3;
                LONG bar_width = data->widths[i] - 37;
                SetAPen(rp, data->draw->dri_Pens[BARDETAILPEN]);
                SetDrMd(rp, JAM1);
                Move(rp, bar_left, text_y);
                Text(rp, (STRPTR)widget_labels[i], strlen(widget_labels[i]));
                RectFill(rp, bar_left, bar_top, bar_left + bar_width - 1, bar_top + 2);
                RectFill(rp, bar_left, bar_top,
                         bar_left + (bar_width * 63 / 100), bar_top + 2);
            }
            x += data->widths[i];
        }
        return 1;
    }

    return DoSuperMethodA(cl, obj, msg);
}

int main(int argc, char **argv)
{
    struct Screen *screen;
    struct DrawInfo *draw;
    struct IClass *cl;
    Object *object = NULL;
    struct Gadget *gadget;
    struct GalleryData *data;
    struct TagItem tags[] = {
        {GA_Width, 0}, {GA_Height, 0}, {GA_Top, 0}, {TAG_DONE, 0}
    };
    ULONG colors[3];
    ULONG seconds = argc > 1 ? atol(argv[1]) : 60;
    ULONG start_seconds, start_micros, now_seconds, now_micros;
    ULONG frame, previous_frame = ~(ULONG)0;
    int total_width = 0, i, result = RETURN_FAIL;
    BOOL attached = FALSE;

    if (!IntuitionBase || ((struct Library *)IntuitionBase)->lib_Version != 50 ||
        ((struct Library *)IntuitionBase)->lib_Revision < 11)
        return RETURN_FAIL;

    CyberGfxBase = OpenLibrary((CONST_STRPTR)"cybergraphics.library", 0);
    if (!CyberGfxBase)
        return RETURN_FAIL;

    screen = LockPubScreen(NULL);
    if (!screen)
        goto close_cgfx;
    draw = GetScreenDrawInfo(screen);
    if (!draw)
        goto unlock_screen;

    GetRGB32(screen->ViewPort.ColorMap, draw->dri_Pens[BARDETAILPEN], 1, colors);
    colors[0] = (((colors[0] >> 24) & 0xFF) << 16) |
                (((colors[1] >> 24) & 0xFF) << 8) |
                ((colors[2] >> 24) & 0xFF) | 0xFF000000UL;

    for (i = 0; i < WIDGET_COUNT; ++i)
        total_width += sb_widget_width(preferred_widths[i], intrinsic_widths[i]);

    cl = MakeClass(NULL, (ClassID)"gadgetclass", NULL, sizeof(struct GalleryData), 0);
    if (!cl)
        goto free_draw;
    cl->cl_Dispatcher.h_Entry = HookEntry;
    cl->cl_Dispatcher.h_SubEntry = (HOOKFUNC)gallery_dispatcher;

    tags[0].ti_Data = total_width;
    tags[1].ti_Data = screen->BarHeight - 2;
    tags[2].ti_Data = 1;
    object = NewObjectA(cl, NULL, tags);
    if (!object)
        goto free_class;

    gadget = (struct Gadget *)object;
    gadget->Flags |= GFLG_RELRIGHT;
    gadget->LeftEdge = -(total_width + 26); /* gap plus native depth gadget */
    gadget->GadgetType |= GTYP_SCRGADGET;
    data = INST_DATA(cl, object);
    data->draw = draw;
    data->mono_rgb = colors[0];
    for (i = 0; i < WIDGET_COUNT; ++i)
        data->widths[i] = sb_widget_width(preferred_widths[i], intrinsic_widths[i]);

    if (!AddScreenBarGadget(screen, gadget, 0))
        goto dispose_object;
    attached = TRUE;

    CurrentTime(&start_seconds, &start_micros);
    result = RETURN_OK;
    for (;;)
    {
        ULONG elapsed;
        CurrentTime(&now_seconds, &now_micros);
        elapsed = (now_seconds - start_seconds) * 1000UL;
        if (now_micros >= start_micros)
            elapsed += (now_micros - start_micros) / 1000UL;
        else
            elapsed -= (start_micros - now_micros) / 1000UL;
        frame = sb_animation_frame(anim_durations, 3, elapsed, TRUE);
        if (frame != previous_frame)
        {
            data->frame = frame;
            if (!RefreshScreenBarGadget(screen, gadget))
            {
                result = RETURN_FAIL;
                break;
            }
            previous_frame = frame;
        }
        if (seconds && now_seconds - start_seconds >= seconds)
            break;
        if (SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
            break;
        Delay(1);
    }

    if (attached)
        RemoveScreenBarGadget(screen, gadget);
    DisposeObject(object);
free_class:
    FreeClass(cl);
free_draw:
    FreeScreenDrawInfo(screen, draw);
unlock_screen:
    UnlockPubScreen(NULL, screen);
close_cgfx:
    CloseLibrary(CyberGfxBase);
    return result;

dispose_object:
    DisposeObject(object);
    goto free_class;
}
