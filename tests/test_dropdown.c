#include <assert.h>
#include "dropdown.h"
int main(void) {
 struct SbDropdown d;
 sb_dropdown_init(&d,0,2);assert(sb_dropdown_commit(&d)==-1);
 sb_dropdown_init(&d,5,2);assert(d.selected==0);
 sb_dropdown_move(&d,4);assert(d.selected==4 && d.first==3);
 sb_dropdown_move(&d,1);assert(d.selected==4);
 sb_dropdown_move(&d,-9);assert(d.selected==0 && d.first==0);
 assert(sb_dropdown_hit(&d,8,4,120,20,4)==0);
 assert(sb_dropdown_hit(&d,8,24,120,20,4)==1);
 assert(sb_dropdown_hit(&d,0,4,120,20,4)==-1);
 assert(sb_dropdown_hit(&d,8,3,120,20,4)==-1);
 assert(sb_dropdown_hit(&d,120,4,120,20,4)==-1);
 assert(sb_dropdown_hit(&d,8,44,120,20,4)==-1);
 sb_dropdown_init(&d,5,0);assert(sb_dropdown_commit(&d)==-1);
 assert(sb_dropdown_key(0x43)==SB_DD_ACCEPT);
 assert(sb_dropdown_key(0x44)==SB_DD_ACCEPT);
 assert(sb_dropdown_key(0xc4)==SB_DD_IGNORE);
 assert(sb_dropdown_key(0x45)==SB_DD_DISMISS);
 return 0;
}
