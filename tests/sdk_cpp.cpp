#include "screenbar.h"
int main() {
 SbClient c;
 if (!sb_client_open(&c)) return 1;
 sb_client_close(&c);
 return 0;
}
