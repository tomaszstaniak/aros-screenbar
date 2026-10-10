#include "widget_visual.h"

unsigned sb_animation_frame(const unsigned short *durations,
                            unsigned frame_count,
                            unsigned elapsed_ms,
                            int loop)
{
    unsigned total = 0;
    unsigned i;

    if (!durations || !frame_count)
        return 0;

    for (i = 0; i < frame_count; ++i)
        total += durations[i];

    if (!total)
        return 0;

    if (loop)
        elapsed_ms %= total;
    else if (elapsed_ms >= total)
        elapsed_ms = total - 1;

    for (i = 0; i < frame_count; ++i)
    {
        if (elapsed_ms < durations[i])
            return i;
        elapsed_ms -= durations[i];
    }

    return frame_count - 1;
}

int sb_widget_width(int preferred_width, int intrinsic_width)
{
    if (preferred_width > 0)
        return preferred_width;

    return intrinsic_width > 0 ? intrinsic_width : 1;
}
