#ifndef SB_POPUP_H
#define SB_POPUP_H
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include "dropdown.h"
#define SB_MAX_SCREENS 32
struct SbPopup {
 struct Window *window;
 struct Screen *screen;
 struct DrawInfo *draw;
 struct SbDropdown state;
 char names[SB_MAX_SCREENS][128];
 int current, row_height, pressed;
 void (*record)(const char *);
};
int sb_popup_open(struct SbPopup *p,struct Screen *screen,struct DrawInfo *draw,int anchor_x,int anchor_y);
void sb_popup_pump(struct SbPopup *p);
void sb_popup_close(struct SbPopup *p);
#endif
