#include <assert.h>
#include <stdio.h>
#include "item_layout.h"
int main(void) {
 struct SbItemLayout l;int widths[]={26,26,60,80,80};
 sb_items_layout(&l,1024,36,570,widths,5,26);
 assert(l.left>=570 && l.left+l.width<=988 && l.count==5 && !l.hidden);
 assert(sb_items_hit(&l,-1)==-1 && sb_items_hit(&l,l.width)==-1);
 assert(sb_items_hit(&l,0)==0 && sb_items_hit(&l,26)==1);
 sb_items_layout(&l,640,36,466,widths,5,26);
 assert(l.left>=466 && l.left+l.width<=604 && l.count==3 && l.hidden==2);
 assert(sb_items_hit(&l,l.width-1)==SB_HIT_OVERFLOW);
 sb_items_layout(&l,100,36,70,widths,5,26);assert(l.width==0 && l.count==0 && l.hidden==5);
 sb_items_layout(&l,640,36,580,widths,5,26);assert(l.width==0 && l.count==0);
 sb_items_layout(&l,640,36,578,widths,5,26);assert(l.count==0 && l.width==26 && l.hidden==5);
 sb_items_layout(&l,640,36,0,widths,0,26);assert(!l.width && !l.hidden);
 struct SbItemTarget old_targets[]={{0,0},{-1,10},{-1,11}};
 struct SbItemTarget same_targets[]={{0,0},{-1,10},{-1,11}};
 struct SbItemTarget replacement[]={{0,0},{-1,12},{-1,11}};
 assert(!sb_targets_changed(old_targets,3,same_targets,3));
 assert(sb_targets_changed(old_targets,3,replacement,3));
 assert(sb_targets_changed(old_targets,3,same_targets,2));
 same_targets[0].builtin=1;assert(sb_targets_changed(old_targets,3,same_targets,3));
 puts("item layout tests passed");return 0;
}
