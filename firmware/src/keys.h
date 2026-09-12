/* Portable key scanner API: stable edges, one action per physical press. */
#ifndef KACHI_KEYS_H
#define KACHI_KEYS_H
#include <stdint.h>
#include "config.h"

typedef struct {
    uint16_t changed_at;
    uint8_t raw;
    uint8_t armed;
} Key;

void keys_reset(uint8_t pressed, uint16_t now);
/* Return physical ID + 1 for a press, or 0 for no action. */
uint8_t keys_scan(uint8_t pressed, uint16_t now, uint8_t accepting);
#endif
