/* Text sequencer: one report at a time, advanced only after successful enqueue. */
#ifndef KACHI_TEXT_H
#define KACHI_TEXT_H
#include <stdint.h>
#include "config.h"
void text_reset(void);
uint8_t text_busy(void);
uint8_t text_start(uint8_t action);
uint8_t text_peek(uint8_t caps_lock, uint8_t *report);
void text_tick(uint16_t now);
void text_accepted(uint16_t now);
#endif
