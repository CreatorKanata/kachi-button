/* Pulse the normally lit LED off; latch expiry so clock wrap cannot retrigger it. */
#include "key_led.h"
static uint8_t pulsing;
static uint16_t started_at;

void key_led_reset(void) { pulsing = 0; }

uint8_t key_led_off(uint8_t pressed_edges, uint16_t now) {
    if (pressed_edges) {
        started_at = now;
        pulsing = 1;
    }
    if (pulsing && (uint16_t)(now - started_at) >= KEY_LED_PULSE_MS)
        pulsing = 0;
    return pulsing;
}
