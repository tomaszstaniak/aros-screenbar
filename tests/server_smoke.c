#include "server.h"
#include "screenbar.h"
#include <proto/exec.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 FILE *log=fopen(argv[1],"w");if(!log)return 1;
 struct SbServer server,duplicate;struct MsgPort *reply=CreateMsgPort();int failed=0;
 if(!reply || !sb_server_open(&server)) {fprintf(log,"FAIL: server requires no running host\n");if(reply)DeleteMsgPort(reply);fclose(log);return 1;}
 if(sb_server_open(&duplicate)) {fprintf(log,"FAIL: duplicate host accepted\n");sb_server_close(&duplicate);failed=1;}
 struct SbWireMessage messages[2];memset(messages,0,sizeof(messages));
 for(int i=0;i<2;i++) {
  messages[i].message.mn_Length=sizeof(messages[i]);messages[i].message.mn_ReplyPort=reply;
  messages[i].magic=SB_MESSAGE_MAGIC;messages[i].request.version=SB_PROTOCOL_VERSION;
  messages[i].request.op=SB_REGISTER;strcpy(messages[i].request.label,"Pending");
  PutMsg(server.port,&messages[i].message);
 }
 sb_server_close(&server);
 for(int i=0;i<2;i++) {
  struct Message *m=GetMsg(reply);
  if(m!=&messages[i].message || messages[i].response.status!=SB_STOPPED) {fprintf(log,"FAIL: queued shutdown reply %d\n",i);failed=1;}
 }
 Forbid();int absent=FindPort((STRPTR)SB_PORT_NAME)==NULL;Permit();
 if(!absent) {fprintf(log,"FAIL: host port still published\n");failed=1;}
 DeleteMsgPort(reply);fprintf(log,failed?"Server shutdown FAILED\n":"Server shutdown passed: duplicate rejected, queued requests replied, port removed\n");fclose(log);return failed;
}
