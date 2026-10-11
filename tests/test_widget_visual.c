#include <assert.h>
#include <stddef.h>
#include "../src/widget_visual.h"

unsigned sb_animation_bar_height(unsigned frame, unsigned column);

int main(void)
{
    const unsigned short durations[] = {100, 200, 300};
    unsigned frame, column;
    unsigned heights[3][4];

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

    for (frame = 0; frame < 3; ++frame)
        for (column = 0; column < 4; ++column)
        {
            heights[frame][column] = sb_animation_bar_height(frame, column);
            assert(heights[frame][column] >= 1);
            assert(heights[frame][column] <= 4);
        }

    for (frame = 0; frame < 3; ++frame)
        for (column = frame + 1; column < 3; ++column)
        {
            unsigned bar;
            int differs = 0;
            for (bar = 0; bar < 4; ++bar)
                if (heights[frame][bar] != heights[column][bar])
                    differs = 1;
            assert(differs);
        }

    return 0;
}
