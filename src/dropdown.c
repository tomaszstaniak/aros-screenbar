#include "dropdown.h"
void sb_dropdown_init(struct SbDropdown *d,int count,int visible) {
 d->count=count>0?count:0; d->visible=visible>0?visible:0;
 if(d->visible>d->count)d->visible=d->count;
 d->selected=d->count && d->visible?0:-1; d->first=0;
}
int sb_dropdown_move(struct SbDropdown *d,int delta) {
 int old=d->selected;
 if(old<0)return 0;
 if(delta>0 && delta>d->count-1-old)d->selected=d->count-1;
 else if(delta<0 && delta<-old)d->selected=0;
 else d->selected+=delta;
 if(d->selected<d->first)d->first=d->selected;
 if(d->selected>=d->first+d->visible)d->first=d->selected-d->visible+1;
 return old!=d->selected;
}
int sb_dropdown_hit(const struct SbDropdown *d,int x,int y,int width,int row_height,int padding) {
 if(row_height<=0 || x<padding || x>=width-padding || y<padding)return -1;
 int row=(y-padding)/row_height;
 if(row>=d->visible || row+d->first>=d->count)return -1;
 return row+d->first;
}
int sb_dropdown_commit(const struct SbDropdown *d) {return d->selected;}

enum SbDropdownKey sb_dropdown_key(unsigned code) {
 switch(code) {case 0x43:case 0x44:return SB_DD_ACCEPT;case 0x45:return SB_DD_DISMISS;
 case 0x4c:return SB_DD_UP;case 0x4d:return SB_DD_DOWN;default:return SB_DD_IGNORE;}
}
