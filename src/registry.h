#ifndef SB_REGISTRY_H
#define SB_REGISTRY_H
#include <stdint.h>
#include "../include/screenbar-protocol.h"
struct SbProviderItem {
 uint32_t token,revision,pending_action;
 uint64_t updated;
 char label[SB_LABEL_SIZE],text[SB_TEXT_SIZE],action_label[SB_LABEL_SIZE];
};
struct SbRegistry {uint64_t session;uint32_t next_token;struct SbProviderItem items[SB_MAX_PROVIDERS];};
void sb_registry_init(struct SbRegistry *r,uint64_t session);
void sb_registry_request(struct SbRegistry *r,const struct SbRequest *q,struct SbResponse *a,uint64_t now);
void sb_registry_expire(struct SbRegistry *r,uint64_t now);
int sb_registry_action(struct SbRegistry *r,uint32_t token);
#endif
