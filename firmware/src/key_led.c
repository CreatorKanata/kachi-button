/* Indicate HID entry for 2 s, then pulse on for presses; no blocking delays. */
#include "key_led.h"
enum { IDLE, START_PENDING, START_LIT, KEY_LIT };
static uint8_t state;
static uint16_t started_at;

void key_led_reset(void) { state = START_PENDING; }

uint8_t key_led_on(uint8_t pressed_edges, uint16_t now) {
    if (state == START_PENDING) {
        state = START_LIT;
        started_at = now;
    }
    if (state == START_LIT) {
        /* Entry indication has priority; presses neither shorten nor extend it. */
        if ((uint16_t)(now - started_at) < HID_START_LED_MS) return 1;
        state = IDLE;
    }
    if (pressed_edges) {
        started_at = now;
        state = KEY_LIT;
    }
    if (state == KEY_LIT && (uint16_t)(now - started_at) >= KEY_LED_PULSE_MS)
        state = IDLE;
    return state == KEY_LIT;
}
