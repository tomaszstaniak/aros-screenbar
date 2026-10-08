#ifndef SB_DROPDOWN_H
#define SB_DROPDOWN_H
struct SbDropdown { int count, visible, selected, first; };
void sb_dropdown_init(struct SbDropdown *d,int count,int visible);
int sb_dropdown_move(struct SbDropdown *d,int delta);
int sb_dropdown_hit(const struct SbDropdown *d,int x,int y,int width,int row_height,int padding);
int sb_dropdown_commit(const struct SbDropdown *d);
enum SbDropdownKey { SB_DD_IGNORE, SB_DD_UP, SB_DD_DOWN, SB_DD_ACCEPT, SB_DD_DISMISS };
enum SbDropdownKey sb_dropdown_key(unsigned code);
#endif
