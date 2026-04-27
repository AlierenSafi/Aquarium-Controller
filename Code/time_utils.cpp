#include "time_utils.h"
#include <stdio.h>

void fmtTime(char* b, uint8_t h, uint8_t m, uint8_t s) {
    snprintf(b, 9, "%02u:%02u:%02u", h, m, s);
}
void fmtDate(char* b, uint8_t d, uint8_t mo, uint16_t yr) {
    snprintf(b, 11, "%02u/%02u/%04u", d, mo, yr);
}
bool schedMatch(uint8_t dm, uint8_t dow,
                uint8_t h, uint8_t m, uint8_t sh, uint8_t sm) {
    return (dm & (1u << dow)) && (h == sh) && (m == sm);
}
