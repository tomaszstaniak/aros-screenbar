#ifndef SB_MODULE_H
#define SB_MODULE_H
enum SbModuleId { SB_MODULE_SCREENS, SB_MODULE_AUDIO, SB_MODULE_CLOCK };
enum SbAction { SB_ACTION_NONE, SB_ACTION_SHOW_SCREENS, SB_ACTION_OPEN_AHI };
enum SbIcon { SB_ICON_NONE, SB_ICON_SCREENS, SB_ICON_AUDIO };
struct SbModuleState { enum SbModuleId id; enum SbIcon icon; int available; char text[32]; };
void sb_modules_update(struct SbModuleState states[3], const char *clock_text);
enum SbAction sb_module_action(enum SbModuleId id);
#endif
