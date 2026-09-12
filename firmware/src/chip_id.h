/* Read the factory 40-bit ID without touching configuration or flash writes. */
#ifndef KACHI_CHIP_ID_H
#define KACHI_CHIP_ID_H
#include <stdint.h>
void chip_id_read(uint8_t *output);
#endif
