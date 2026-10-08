#include <assert.h>
#include <stdio.h>
#include "../examples/native_probe_wait.h"
struct Replies { void *items[3]; unsigned next; };
static void *next_reply(void *context)
{
    struct Replies *replies = context;
    assert(replies->next < 3);
    return replies->items[replies->next++];
}
int main(void)
{
    int request, unexpected;
    struct Replies normal = {{&request, 0, 0}, 0};
    struct Replies foreign = {{&unexpected, &request, 0}, 0};
    assert(sb_native_wait_reply(&request, &normal, next_reply) == 0);
    assert(normal.next == 1);
    assert(sb_native_wait_reply(&request, &foreign, next_reply) == -1);
    assert(foreign.next == 2); /* Never release the request after a foreign reply. */
    puts("native probe reply lifetime tests passed");
    return 0;
}
