#ifndef SB_ITEM_LAYOUT_H
#define SB_ITEM_LAYOUT_H
#include <stdint.h>
struct SbItemTarget {int builtin;uint32_t token;};
int sb_targets_changed(const struct SbItemTarget *a,int ac,const struct SbItemTarget *b,int bc);
#define SB_MAX_ITEMS 11
#define SB_HIT_OVERFLOW (-2)
struct SbItemLayout {int left,width,count,hidden,overflow_width;int widths[SB_MAX_ITEMS];};
void sb_items_layout(struct SbItemLayout *l,int screen_width,int right_reserve,int title_right,const int *widths,int count,int overflow_width);
int sb_items_hit(const struct SbItemLayout *l,int x);
#endif
