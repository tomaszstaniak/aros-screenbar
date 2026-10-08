#include "registry.h"
#include <string.h>
static int valid_text(const char *s,unsigned size) {
 for(unsigned i=0;i<size;i++) {unsigned char c=(unsigned char)s[i];if(!c)return 1;if(c<32 || c>126)return 0;}return 0;
}
void sb_registry_init(struct SbRegistry *r,uint64_t session) {memset(r,0,sizeof(*r));r->session=session;r->next_token=1;}
void sb_registry_expire(struct SbRegistry *r,uint64_t now) {
 for(int i=0;i<SB_MAX_PROVIDERS;i++)if(r->items[i].token && now>=r->items[i].updated && now-r->items[i].updated>=SB_LEASE_SECONDS)memset(&r->items[i],0,sizeof(r->items[i]));
}
void sb_registry_request(struct SbRegistry *r,const struct SbRequest *q,struct SbResponse *a,uint64_t now) {
 memset(a,0,sizeof(*a));a->session=r->session;
 if(q->version!=SB_PROTOCOL_VERSION) {a->status=SB_VERSION;return;}
 if(q->op<SB_REGISTER || q->op>SB_UNREGISTER) {a->status=SB_INVALID;return;}
 if(q->op!=SB_UNREGISTER && (!valid_text(q->label,sizeof(q->label)) || !q->label[0] || !valid_text(q->text,sizeof(q->text)) || !valid_text(q->action_label,sizeof(q->action_label)))) {a->status=SB_INVALID;return;}
 struct SbProviderItem *item=NULL;
 if(q->op==SB_REGISTER) {
  for(int i=0;i<SB_MAX_PROVIDERS;i++)if(!r->items[i].token) {item=&r->items[i];break;}
  if(!item || !r->next_token) {a->status=SB_FULL;return;}
  memset(item,0,sizeof(*item));item->token=r->next_token++;
 } else {
  if(q->session!=r->session || !q->token) {a->status=SB_STALE;return;}
  for(int i=0;i<SB_MAX_PROVIDERS;i++)if(r->items[i].token==q->token) {item=&r->items[i];break;}
  if(!item) {a->status=SB_STALE;return;}
  if(q->op==SB_UNREGISTER) {memset(item,0,sizeof(*item));return;}
  if(q->revision<=item->revision) {a->status=SB_REVISION;return;}
 }
 item->revision=q->revision;item->updated=now;
 memcpy(item->label,q->label,sizeof(item->label));memcpy(item->text,q->text,sizeof(item->text));
 memcpy(item->action_label,q->action_label,sizeof(item->action_label));
 a->token=item->token;a->action=item->pending_action;item->pending_action=0;
}
int sb_registry_action(struct SbRegistry *r,uint32_t token) {
 if(!token)return 0;
 for(int i=0;i<SB_MAX_PROVIDERS;i++)if(r->items[i].token==token && r->items[i].action_label[0]) {r->items[i].pending_action=SB_ACTION_PRIMARY;return 1;}
 return 0;
}
