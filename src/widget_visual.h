#ifndef SB_WIDGET_VISUAL_H
#define SB_WIDGET_VISUAL_H

unsigned sb_animation_frame(const unsigned short *durations,
                            unsigned frame_count,
                            unsigned elapsed_ms,
                            int loop);
unsigned sb_animation_bar_height(unsigned frame, unsigned column);
int sb_widget_width(int preferred_width, int intrinsic_width);

#endif
