#include "module.h"
#include <stdio.h>
void sb_modules_update(struct SbModuleState states[3], const char *clock_text) {
 for (int i=0;i<3;i++) { states[i].id=(enum SbModuleId)i; states[i].available=1; states[i].text[0]=0; states[i].icon=SB_ICON_NONE; }
 states[0].icon=SB_ICON_SCREENS; states[1].icon=SB_ICON_AUDIO;
 snprintf(states[2].text,sizeof(states[2].text),"%s",clock_text?clock_text:"");
}
enum SbAction sb_module_action(enum SbModuleId id) {
 switch(id) { case SB_MODULE_SCREENS:return SB_ACTION_SHOW_SCREENS; case SB_MODULE_AUDIO:return SB_ACTION_OPEN_AHI; default:return SB_ACTION_NONE; }
}
