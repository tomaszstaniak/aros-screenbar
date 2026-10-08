#ifndef SB_SERVER_H
#define SB_SERVER_H
#include <exec/ports.h>
#include <devices/timer.h>
#include "registry.h"
struct SbServer {struct MsgPort *port,*timer_port;struct timerequest *timer;uint32_t frequency;struct SbRegistry registry;};
int sb_server_open(struct SbServer *s);
uint64_t sb_server_now(struct SbServer *s);
void sb_server_pump(struct SbServer *s,uint64_t now);
void sb_server_close(struct SbServer *s);
#endif
