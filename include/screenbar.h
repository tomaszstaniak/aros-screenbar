#ifndef SCREENBAR_H
#define SCREENBAR_H
#include <exec/ports.h>
#include "screenbar-protocol.h"
#define SB_PORT_NAME "AROS.ScreenBar.1"
#define SB_MESSAGE_MAGIC 0x53423100UL
struct SbWireMessage {struct Message message;uint32_t magic;struct SbRequest request;struct SbResponse response;};
struct SbClient {struct MsgPort *reply;uint64_t session;uint32_t token,revision;char action_label[SB_LABEL_SIZE];};
#ifdef __cplusplus
extern "C" {
#endif
int sb_client_open(struct SbClient *client);
void sb_client_close(struct SbClient *client);
int sb_client_register(struct SbClient *client,const char *label,const char *text,const char *action_label);
int sb_client_update(struct SbClient *client,const char *label,const char *text,unsigned *action);
int sb_client_unregister(struct SbClient *client);
#ifdef __cplusplus
}
#endif
#endif
