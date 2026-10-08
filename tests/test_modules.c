#include <assert.h>
#include <string.h>
#include "module.h"
int main(void) {
 struct SbModuleState states[3];
 sb_modules_update(states,"13:08");
 assert(states[2].id==SB_MODULE_CLOCK && !strcmp(states[2].text,"13:08"));
 sb_modules_update(states,"13:09");assert(!strcmp(states[2].text,"13:09"));
 assert(sb_module_action(SB_MODULE_SCREENS)==SB_ACTION_SHOW_SCREENS);
 assert(sb_module_action(SB_MODULE_AUDIO)==SB_ACTION_OPEN_AHI);
 assert(sb_module_action(SB_MODULE_CLOCK)==SB_ACTION_NONE);
 assert(sb_module_action((enum SbModuleId)99)==SB_ACTION_NONE);
 sb_modules_update(states,"1234567890123456789012345678901234567890");
 assert(strlen(states[2].text)==31);
 return 0;
}
