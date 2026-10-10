#include <assert.h>
#include <stddef.h>
#include "../src/widget_visual.h"

int main(void)
{
    const unsigned short durations[] = {100, 200, 300};

    assert(sb_animation_frame(durations, 3, 0, 1) == 0);
    assert(sb_animation_frame(durations, 3, 99, 1) == 0);
    assert(sb_animation_frame(durations, 3, 100, 1) == 1);
    assert(sb_animation_frame(durations, 3, 299, 1) == 1);
    assert(sb_animation_frame(durations, 3, 300, 1) == 2);
    assert(sb_animation_frame(durations, 3, 600, 1) == 0);
    assert(sb_animation_frame(durations, 3, 900, 0) == 2);
    assert(sb_animation_frame(NULL, 0, 100, 1) == 0);
    assert(sb_widget_width(96, 30) == 96);
    assert(sb_widget_width(0, 30) == 30);
    assert(sb_widget_width(-1, 30) == 30);
    return 0;
}
