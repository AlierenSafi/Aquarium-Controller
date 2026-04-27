#pragma once
#include <stdint.h>

void fmtTime(char* buf, uint8_t h, uint8_t m, uint8_t s); /* buf>=9  */
void fmtDate(char* buf, uint8_t d, uint8_t mo, uint16_t yr); /* buf>=11 */
bool schedMatch(uint8_t dayMask, uint8_t dow,
                uint8_t h, uint8_t m, uint8_t sh, uint8_t sm);
