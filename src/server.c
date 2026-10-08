#include "server.h"
#include "../include/screenbar.h"
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <string.h>
int sb_server_open(struct SbServer *s) {
 memset(s,0,sizeof(*s));
 struct MsgPort *timer_port=CreateMsgPort();if(!timer_port)return 0;
 struct timerequest *timer=(struct timerequest *)CreateIORequest(timer_port,sizeof(*timer));
 if(!timer) {DeleteMsgPort(timer_port);return 0;}
 if(OpenDevice((STRPTR)"timer.device",UNIT_MICROHZ,(struct IORequest *)timer,0)) {DeleteIORequest((struct IORequest *)timer);DeleteMsgPort(timer_port);return 0;}
 struct Device *TimerBase=timer->tr_node.io_Device;struct EClockVal ticks;
 s->frequency=ReadEClock(&ticks);uint64_t session=((uint64_t)ticks.ev_hi<<32)|ticks.ev_lo;
 s->timer=timer;s->timer_port=timer_port;
 sb_registry_init(&s->registry,session);
 s->port=CreateMsgPort();if(!s->port) {sb_server_close(s);return 0;}
 s->port->mp_Node.ln_Name=(char *)SB_PORT_NAME;
 Forbid();int exists=FindPort((STRPTR)SB_PORT_NAME)!=NULL;
 if(!exists)AddPort(s->port);
 Permit();
 if(exists) {DeleteMsgPort(s->port);s->port=NULL;sb_server_close(s);return 0;}return 1;
}
static void reply(struct SbServer *s,struct Message *msg,uint64_t now,int stopped) {
 if(msg->mn_Length==sizeof(struct SbWireMessage)) {
  struct SbWireMessage *m=(struct SbWireMessage *)msg;
  memset(&m->response,0,sizeof(m->response));
  if(m->magic!=SB_MESSAGE_MAGIC)m->response.status=SB_INVALID;
  else if(stopped)m->response.status=SB_STOPPED;
  else sb_registry_request(&s->registry,&m->request,&m->response,now);
 }
 ReplyMsg(msg);
}
void sb_server_pump(struct SbServer *s,uint64_t now) {
 sb_registry_expire(&s->registry,now);
 struct Message *m;
 for(int i=0;i<32 && (m=GetMsg(s->port));i++)reply(s,m,now,0);
}
uint64_t sb_server_now(struct SbServer *s) {
 struct Device *TimerBase=s->timer->tr_node.io_Device;struct EClockVal ticks;
 ReadEClock(&ticks);uint64_t value=((uint64_t)ticks.ev_hi<<32)|ticks.ev_lo;
 return s->frequency?value/s->frequency:0;
}
void sb_server_close(struct SbServer *s) {
 if(s->port) {
  RemPort(s->port);
  struct Message *m;while((m=GetMsg(s->port)))reply(s,m,0,1);
  DeleteMsgPort(s->port);s->port=NULL;
 }
 if(s->timer) {CloseDevice((struct IORequest *)s->timer);DeleteIORequest((struct IORequest *)s->timer);s->timer=NULL;}
 if(s->timer_port) {DeleteMsgPort(s->timer_port);s->timer_port=NULL;}
}
