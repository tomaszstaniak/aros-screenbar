#include "item_layout.h"
#include <string.h>
void sb_items_layout(struct SbItemLayout *l,int screen_width,int right_reserve,int title_right,const int *widths,int count,int overflow_width) {
 memset(l,0,sizeof(*l));
 if(count<0)count=0;
 if(count>SB_MAX_ITEMS)count=SB_MAX_ITEMS;
 if(right_reserve<0)right_reserve=0;
 if(title_right<0)title_right=0;
 int right=screen_width-right_reserve,available=right-title_right,total=0;
 if(overflow_width<1)overflow_width=1;
 for(int i=0;i<count;i++)if(widths[i]>0)total+=widths[i];
 if(available<=0 || (total>available && overflow_width>available)) {l->hidden=count;l->left=right;return;}
 int limit=total>available?available-overflow_width:available;
 for(int i=0;i<count;i++) {
  int w=widths[i]>0?widths[i]:1;if(l->width+w>limit)break;
  l->widths[l->count++]=w;l->width+=w;
 }
 l->hidden=count-l->count;
 if(l->hidden) {l->overflow_width=overflow_width;l->width+=overflow_width;}
 l->left=right-l->width;
}
int sb_items_hit(const struct SbItemLayout *l,int x) {
 if(x<0 || x>=l->width)return -1;
 for(int i=0;i<l->count;i++) {if(x<l->widths[i])return i;x-=l->widths[i];}
 return l->hidden?SB_HIT_OVERFLOW:-1;
}

int sb_targets_changed(const struct SbItemTarget *a,int ac,const struct SbItemTarget *b,int bc) {
 if(ac!=bc)return 1;
 for(int i=0;i<ac;i++)if(a[i].builtin!=b[i].builtin || a[i].token!=b[i].token)return 1;
 return 0;
}
