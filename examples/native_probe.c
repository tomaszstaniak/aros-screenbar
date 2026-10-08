/* Read-only feasibility probe for Decoration's experimental title-child path.
 * A returned class is observed only: never dereferenced, retained or instantiated.
 */
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <string.h>
#include "native_probe_wait.h"

#define TITLE_CLASS_QUERY 0x0F0E

static void *next_reply(void *context)
{
    struct MsgPort *port = context;
    WaitPort(port);
    return GetMsg(port);
}

static int query_class(BPTR log, struct MsgPort *reply)
{
    struct DecoratorMessage request;
    struct MsgPort *service;
    memset(&request, 0, sizeof(request));
    request.dm_Message.mn_ReplyPort = reply;
    request.dm_Message.mn_Length = sizeof(request);
    request.dm_Message.mn_Magic = TITLE_CLASS_QUERY;
    request.dm_Message.mn_Version = DECORATOR_VERSION;

    /* Find and enqueue as one cooperative Exec operation. The stack message
     * stays alive until replied. There is deliberately no timeout/free path.
     */
    Forbid();
    service = FindPort((CONST_STRPTR)"Decorator");
    if (service) PutMsg(service, (struct Message *)&request);
    Permit();
    if (!service) {
        FPuts(log, (CONST_STRPTR)"Decorator port absent; no native object registered.\n");
        return 0;
    }
    if (sb_native_wait_reply(&request, reply, next_reply) != 0) {
        FPuts(log, (CONST_STRPTR)"Unexpected reply; probe failed.\n");
        return -1;
    }
    if (request.dm_Class) {
        FPuts(log, (CONST_STRPTR)"Title-child class query replied with a nonzero class.\n");
        return 1;
    }
    FPuts(log, (CONST_STRPTR)"Query replied without a class; no native object registered.\n");
    return 0;
}

int main(int argc, char **argv)
{
    struct MsgPort *reply;
    BPTR log;
    int close_log = 0, found = 0, i;
    if (argc > 2) return RETURN_ERROR;
    log = argc == 2 ? Open((CONST_STRPTR)argv[1], MODE_NEWFILE) : Output();
    if (!log) return RETURN_FAIL;
    close_log = argc == 2;
    FPuts(log, (CONST_STRPTR)"ScreenBar native attachment audit probe\n");
    FPrintf(log, (CONST_STRPTR)"Intuition version %ld revision %ld\n",
            (LONG)((struct Library *)IntuitionBase)->lib_Version,
            (LONG)((struct Library *)IntuitionBase)->lib_Revision);
    reply = CreateMsgPort();
    if (!reply) {
        FPuts(log, (CONST_STRPTR)"Cannot create reply port.\n");
        if (close_log) Close(log);
        return RETURN_FAIL;
    }
    for (i = 0; i < 3; ++i) {
        int result = query_class(log, reply);
        if (result < 0) { found = -1; break; }
        found += result;
    }
    DeleteMsgPort(reply);
    FPrintf(log, (CONST_STRPTR)"Successful class queries: %ld/3\n", (LONG)found);
    FPuts(log, (CONST_STRPTR)"No TITLECHILD registration was sent; no screen gadget or overlay created.\n");
    FPuts(log, (CONST_STRPTR)"A class reply proves discovery only, not input, detach or reopen safety.\n");
    FPuts(log, (CONST_STRPTR)"Native attachment remains disabled pending a verified lifecycle/input adapter.\n");
    Flush(log);
    if (close_log) Close(log);
    return found == 3 ? RETURN_OK : RETURN_WARN;
}
