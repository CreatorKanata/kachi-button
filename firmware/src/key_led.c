/* Nonblocking HID entry, verified-save indication, and physical key feedback. */
#include "key_led.h"
enum { IDLE, START_PENDING, START_LIT, KEY_LIT, SAVE_LIT };
static uint8_t state;
static uint16_t started_at;

void key_led_reset(void) { state = START_PENDING; }

void key_led_save_finished(uint8_t ok, uint16_t now) {
    /* Failed writes never trigger success feedback; another success restarts it. */
    if (ok) {
        state = SAVE_LIT;
        started_at = now;
    }
}

uint8_t key_led_on(uint8_t pressed_edges, uint16_t now) {
    if (state == SAVE_LIT) {
        /* Save completion overrides entry/key pulses without blocking typing. */
        if ((uint16_t)(now - started_at) < SAVE_LED_MS) return 1;
        state = IDLE;
    }
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
