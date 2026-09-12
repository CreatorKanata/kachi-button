/* HID entry indication followed by normally-off, nonblocking key feedback. */
#ifndef KACHI_KEY_LED_H
#define KACHI_KEY_LED_H
#include <stdint.h>
#include "config.h"
void key_led_reset(void);
/* First poll after reset starts HID entry; call only in active normal mode. */
uint8_t key_led_on(uint8_t pressed_edges, uint16_t now);
#endif
