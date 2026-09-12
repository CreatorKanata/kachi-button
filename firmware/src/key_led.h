/* Nonblocking feedback for one debounced physical press, independent of macros. */
#ifndef KACHI_KEY_LED_H
#define KACHI_KEY_LED_H
#include <stdint.h>
#include "config.h"
void key_led_reset(void);
uint8_t key_led_off(uint8_t pressed_edges, uint16_t now);
#endif
