#ifndef SCREENBAR_NATIVE_PROBE_WAIT_H
#define SCREENBAR_NATIVE_PROBE_WAIT_H
/* The caller owns expected until the matching reply has been consumed. */
static int sb_native_wait_reply(void *expected, void *context,
                               void *(*next_reply)(void *))
{
    int result = 0;
    void *reply;
    while ((reply = next_reply(context)) != expected)
        result = -1;
    return result;
}
#endif
