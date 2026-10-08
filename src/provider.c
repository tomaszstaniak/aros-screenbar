#include "../include/screenbar.h"
#include <proto/exec.h>
#include <string.h>
static int copy_text(char *out,unsigned size,const char *text) {
 if(!text || strlen(text)>=size)return 0;
 for(const unsigned char *p=(const unsigned char *)text;*p;p++)if(*p<32 || *p>126)return 0;
 strcpy(out,text);return 1;
}
int sb_client_open(struct SbClient *c) {memset(c,0,sizeof(*c));c->reply=CreateMsgPort();return c->reply!=NULL;}
static int transact(struct SbClient *c,unsigned op,const char *label,const char *text,unsigned *action) {
 if(action)*action=0;
 if(!c->reply)return SB_INVALID;
 struct SbWireMessage m;memset(&m,0,sizeof(m));
 m.message.mn_Length=sizeof(m);m.message.mn_ReplyPort=c->reply;m.magic=SB_MESSAGE_MAGIC;
 memcpy(m.request.action_label,c->action_label,sizeof(m.request.action_label));
 m.request.version=SB_PROTOCOL_VERSION;m.request.op=op;m.request.session=c->session;m.request.token=c->token;
 if(op!=SB_UNREGISTER && (!copy_text(m.request.label,sizeof(m.request.label),label) || !m.request.label[0] || !copy_text(m.request.text,sizeof(m.request.text),text)))return SB_INVALID;
 if(op==SB_UPDATE) {
  if(c->revision==UINT32_MAX)return SB_REVISION;
  m.request.revision=c->revision+1;
 }
 /* FindPort returns a borrowed pointer: do not let shutdown interleave the send. */
 Forbid();struct MsgPort *host=FindPort((STRPTR)SB_PORT_NAME);
 if(host)PutMsg(host,&m.message);
 Permit();
 if(!host) {c->token=0;c->session=0;return SB_ABSENT;}
 /* This stack message and its port stay alive until the host replies. */
 WaitPort(c->reply);GetMsg(c->reply);
 int status=m.response.status;
 if(status==SB_OK) {
  if(op==SB_UNREGISTER) {c->token=0;c->session=0;c->revision=0;}
  else {c->session=m.response.session;c->token=m.response.token;c->revision=m.request.revision;}
  if(action)*action=m.response.action;
 } else if(status==SB_STALE || status==SB_STOPPED) {c->token=0;c->session=0;c->revision=0;}
 return status;
}
int sb_client_register(struct SbClient *c,const char *label,const char *text,const char *action_label) {
 if(c->token)return SB_INVALID;
 if(!copy_text(c->action_label,sizeof(c->action_label),action_label))return SB_INVALID;
 return transact(c,SB_REGISTER,label,text,NULL);
}
int sb_client_update(struct SbClient *c,const char *label,const char *text,unsigned *action) {return transact(c,SB_UPDATE,label,text,action);}
int sb_client_unregister(struct SbClient *c) {return c->token?transact(c,SB_UNREGISTER,NULL,NULL,NULL):SB_OK;}
void sb_client_close(struct SbClient *c) {if(c->reply) {sb_client_unregister(c);DeleteMsgPort(c->reply);c->reply=NULL;}}
