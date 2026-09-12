/* Require all three keys at startup and continuously for the configured hold. */
#include "boot_gesture.h"
static uint8_t state;
static uint16_t started_at;

void boot_gesture_init(uint8_t pressed, uint16_t now) {
    state = pressed == ALL_KEYS_MASK ? BOOT_HOLDING : BOOT_NORMAL;
    started_at = now;
}

uint8_t boot_gesture_poll(uint8_t pressed, uint16_t now) {
    if (state == BOOT_HOLDING) {
        /* Any observed release cancels; later presses cannot rearm it. */
        if (pressed != ALL_KEYS_MASK)
            state = BOOT_NORMAL;
        else if ((uint16_t)(now - started_at) >= BOOT_HOLD_MS)
            state = BOOT_WAIT;
    }
    /* BOOT_WAIT is latched until power/reset, regardless of time or keys. */
    return state;
}
