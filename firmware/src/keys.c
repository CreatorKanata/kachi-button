/* PCB v1 key scanning: reject bounce, held startup keys and busy-time presses. */
#include "keys.h"

static Key keys[KEY_COUNT];

void keys_reset(uint8_t pressed, uint16_t now) {
    uint8_t i;
    for (i = 0; i < KEY_COUNT; ++i) {
        keys[i].raw = (pressed >> i) & 1;
        keys[i].changed_at = now;
        keys[i].armed = 0;
    }
}

uint8_t keys_scan(uint8_t pressed, uint16_t now, uint8_t accepting) {
    uint8_t i, action = 0;
    for (i = 0; i < KEY_COUNT; ++i) {
        uint8_t raw = (pressed >> i) & 1;
        if (raw != keys[i].raw) {
            keys[i].raw = raw;
            keys[i].changed_at = now;
        }
        /* Unsigned subtraction also works across the 16-bit clock wrap. */
        if ((uint16_t)(now - keys[i].changed_at) < DEBOUNCE_MS)
            continue;
        if (!raw) {
            keys[i].armed = 1;
        } else if (keys[i].armed) {
            keys[i].armed = 0;
            /* One action at a time; simultaneous presses use physical order. */
            if (accepting && !action)
                action = i + 1;
        }
    }
    return action;
}
