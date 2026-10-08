#include "screenbar.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(int argc,char **argv) {
 int seconds=60;const char *label="Counter";FILE *log=NULL;
 for(int i=1;i<argc;i++) {
  if(!strcmp(argv[i],"--seconds") && i+1<argc)seconds=atoi(argv[++i]);
  else if(!strcmp(argv[i],"--label") && i+1<argc)label=argv[++i];
  else if(!strcmp(argv[i],"--log") && i+1<argc) {log=fopen(argv[++i],"w");if(!log)return 1;}
  else {fprintf(stderr,"Usage: ScreenBarCounter [--seconds N] [--label NAME] [--log PATH]\n");return 2;}
 }
 struct SbClient c;if(!sb_client_open(&c))return 1;
 time_t start=time(NULL),reset=start;int failed=0;
 while(seconds<=0 || time(NULL)-start<seconds) {
  if(SetSignal(0,SIGBREAKF_CTRL_C)&SIGBREAKF_CTRL_C)break;
  char text[SB_TEXT_SIZE];snprintf(text,sizeof(text),"%ld",(long)(time(NULL)-reset));unsigned action=0;
  int status=c.token?sb_client_update(&c,label,text,&action):sb_client_register(&c,label,text,"Reset");
  if(status==SB_OK && action==SB_ACTION_PRIMARY) {reset=time(NULL);if(log) {fprintf(log,"Reset received\n");fflush(log);}}
  if(status==SB_OK && c.revision==0 && log) {fprintf(log,"Registered token=%u session=%llu\n",c.token,(unsigned long long)c.session);fflush(log);}
  if(status!=SB_OK && status!=SB_ABSENT && status!=SB_STALE && status!=SB_STOPPED && status!=SB_FULL) {failed=1;break;}
  Delay(10);
 }
 sb_client_close(&c);if(log) {fprintf(log,failed?"Provider error\n":"Provider clean exit\n");fclose(log);}return failed;
}
