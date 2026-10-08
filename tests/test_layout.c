#include <assert.h>
#include "layout.h"
int main(void) {
    assert(sb_left(1024,144,32)==848);
    assert(sb_left(100,144,32)==0);
    assert(sb_hit(0,28)==SB_SCREENS);
    assert(sb_hit(28,28)==SB_VOLUME);
    assert(sb_hit(56,28)==SB_CLOCK);
    assert(sb_hit(-1,28)==SB_NONE);
    assert(sb_cell_hit(107,107,26)==-1);
    assert(sb_cell_hit(-1,107,26)==-1);
    assert(sb_cell_hit(52,107,26)==2);
    return 0;
}
