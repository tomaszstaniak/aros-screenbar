#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "registry.h"
static struct SbRequest request(unsigned op) {
 struct SbRequest q={0};q.version=SB_PROTOCOL_VERSION;q.op=op;
 strcpy(q.label,"Counter");strcpy(q.action_label,"Reset");strcpy(q.text,"1");return q;
}
int main(void) {
 struct SbRegistry r;sb_registry_init(&r,123);
 struct SbRequest q=request(SB_REGISTER);struct SbResponse a;
 sb_registry_request(&r,&q,&a,10);assert(a.status==SB_OK && a.session==123 && a.token);
 assert(!strcmp(r.items[0].action_label,"Reset"));unsigned token=a.token;q.label[0]='X';assert(!strcmp(r.items[0].label,"Counter"));
 q=request(SB_UPDATE);q.session=122;q.token=token;q.revision=1;
 sb_registry_request(&r,&q,&a,11);assert(a.status==SB_STALE);
 q.session=123;strcpy(q.text,"2");sb_registry_request(&r,&q,&a,11);
 assert(a.status==SB_OK && !strcmp(r.items[0].text,"2"));
 strcpy(q.text,"old");sb_registry_request(&r,&q,&a,12);
 assert(a.status==SB_REVISION && !strcmp(r.items[0].text,"2"));
 assert(sb_registry_action(&r,token));q.revision=2;sb_registry_request(&r,&q,&a,12);
 assert(a.action==SB_ACTION_PRIMARY);q.revision=3;sb_registry_request(&r,&q,&a,13);assert(!a.action);
 sb_registry_expire(&r,17);assert(r.items[0].token==token);
 sb_registry_expire(&r,18);assert(!r.items[0].token && !sb_registry_action(&r,token));
 sb_registry_request(&r,&q,&a,19);assert(a.status==SB_STALE);
 q=request(SB_REGISTER);for(int i=0;i<SB_MAX_PROVIDERS;i++) {sb_registry_request(&r,&q,&a,20);assert(a.status==SB_OK);}
 sb_registry_request(&r,&q,&a,20);assert(a.status==SB_FULL);
 q=request(SB_UNREGISTER);q.session=123;q.token=r.items[0].token;
 sb_registry_request(&r,&q,&a,21);assert(a.status==SB_OK && !r.items[0].token);
 q=request(SB_REGISTER);memset(q.text,'x',sizeof(q.text));sb_registry_request(&r,&q,&a,21);assert(a.status==SB_INVALID);
 q=request(SB_REGISTER);q.label[0]=(char)0xff;sb_registry_request(&r,&q,&a,21);assert(a.status==SB_INVALID);
 q=request(SB_REGISTER);q.version=99;sb_registry_request(&r,&q,&a,21);assert(a.status==SB_VERSION);
 q=request(99);sb_registry_request(&r,&q,&a,21);assert(a.status==SB_INVALID);
 puts("registry tests passed");return 0;
}
