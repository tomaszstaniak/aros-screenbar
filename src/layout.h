#ifndef SCREENBAR_LAYOUT_H
#define SCREENBAR_LAYOUT_H
/* Leave the native screen-depth gadget outside the overlay. */
enum { SB_NONE, SB_SCREENS, SB_VOLUME, SB_CLOCK };
static inline int sb_left(int screen_width, int width, int reserve) {
    int x = screen_width - width - reserve;
    return x > 0 ? x : 0;
}
static inline int sb_hit(int x, int cell) {
    if (x < 0 || cell <= 0) return SB_NONE;
    if (x < cell) return SB_SCREENS;
    if (x < cell * 2) return SB_VOLUME;
    return SB_CLOCK;
}
static inline int sb_cell_hit(int x,int width,int cell) {
 if(x<0 || x>=width || cell<=0)return -1;
 return x<cell?0:(x<2*cell?1:2);
}
#endif
