/* Nonblocking normal-mode LED feedback for HID entry, saves, and key presses. */
#ifndef KACHI_KEY_LED_H
#define KACHI_KEY_LED_H
#include <stdint.h>
#include "config.h"
void key_led_reset(void);
/* Foreground only: notify after storage verification with a fresh timestamp. */
void key_led_save_finished(uint8_t ok, uint16_t now);
/* First poll after reset starts HID entry; call only in active normal mode. */
uint8_t key_led_on(uint8_t pressed_edges, uint16_t now);
#endif
