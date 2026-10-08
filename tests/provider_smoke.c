#include "screenbar.h"
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 FILE *log=fopen(argv[1],"w");if(!log)return 1;
 struct SbClient a,b;int opened_a=sb_client_open(&a),opened_b=sb_client_open(&b),failed=0;
 #define CHECK(expr) do {if(!(expr)) {fprintf(log,"FAIL: %s\n",#expr);failed=1;goto done;} else fprintf(log,"PASS: %s\n",#expr);fflush(log);} while(0)
 CHECK(opened_a && opened_b);
 CHECK(sb_client_register(&a,"LeaseProbe","1","Reset")==SB_OK);
 CHECK(sb_client_register(&b,"SDKProbe","2","")==SB_OK);
 CHECK(a.token!=b.token && a.session==b.session);
 unsigned action=99;CHECK(sb_client_update(&a,"LeaseProbe","3",&action)==SB_OK && action==0);
 CHECK(sb_client_register(&a,"Duplicate","4","Changed")==SB_INVALID);
 CHECK(!strcmp(a.action_label,"Reset"));
 char longtext[SB_TEXT_SIZE+1];memset(longtext,'x',sizeof(longtext)-1);longtext[sizeof(longtext)-1]=0;
 CHECK(sb_client_update(&a,"LeaseProbe",longtext,&action)==SB_INVALID);
 CHECK(sb_client_unregister(&b)==SB_OK && !b.token);
 Delay(6*50);
 CHECK(sb_client_update(&a,"LeaseProbe","4",&action)==SB_STALE && !a.token);
 CHECK(sb_client_register(&a,"LeaseProbe","5","Reset")==SB_OK);
 CHECK(sb_client_unregister(&a)==SB_OK && !a.token);
 done:
 if(opened_b)sb_client_close(&b);
 if(opened_a)sb_client_close(&a);
 fprintf(log,failed?"SDK smoke FAILED\n":"SDK smoke passed\n");fclose(log);return failed;
}
