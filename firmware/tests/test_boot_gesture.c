/* Prove startup-only activation, exact hold duration, cancellation and wrap. */
#include <assert.h>
#include <stdio.h>
#include "boot_gesture.h"

int main(void) {
    uint8_t mask;
    uint32_t elapsed;
    for (mask = 0; mask < ALL_KEYS_MASK; ++mask) {
        boot_gesture_init(mask, 0);
        assert(boot_gesture_poll(ALL_KEYS_MASK, BOOT_HOLD_MS) == BOOT_NORMAL);
    }
    boot_gesture_init(ALL_KEYS_MASK, 100);
    assert(boot_gesture_poll(ALL_KEYS_MASK, 100) == BOOT_HOLDING);
    assert(boot_gesture_poll(ALL_KEYS_MASK, 100 + BOOT_HOLD_MS - 1) == BOOT_HOLDING);
    assert(boot_gesture_poll(ALL_KEYS_MASK, 100 + BOOT_HOLD_MS) == BOOT_WAIT);
    boot_gesture_init(ALL_KEYS_MASK, 0);
    assert(boot_gesture_poll(3, BOOT_HOLD_MS - 1) == BOOT_NORMAL);
    assert(boot_gesture_poll(ALL_KEYS_MASK, BOOT_HOLD_MS * 2) == BOOT_NORMAL);
    boot_gesture_init(ALL_KEYS_MASK, 0);
    assert(boot_gesture_poll(0, BOOT_HOLD_MS) == BOOT_NORMAL);
    boot_gesture_init(ALL_KEYS_MASK, 65000);
    assert(boot_gesture_poll(ALL_KEYS_MASK, (uint16_t)(65000u + BOOT_HOLD_MS)) == BOOT_WAIT);
    /* Stay armed for a simulated hour, across wraps and arbitrary releases. */
    for (elapsed = 0; elapsed < 3600000UL; elapsed += 37) {
        assert(boot_gesture_poll((uint8_t)(elapsed % 8), (uint16_t)elapsed) == BOOT_WAIT);
    }
    boot_gesture_init(0, 0);
    assert(boot_gesture_poll(0, BOOT_HOLD_MS) == BOOT_NORMAL);
    puts("PASS: boot gesture startup-only, 2-second boundary, early release, no rearm, wrap, latched one-hour wait, power-up reset");
    return 0;
}
