/* Verify exact off-pulse boundaries, repeat presses, lifecycle reset and wrap. */
#include <assert.h>
#include <stdio.h>
#include "key_led.h"
int main(void) {
    key_led_reset();
    assert(!key_led_off(0, 0));
    assert(key_led_off(1, 100));
    assert(key_led_off(0, 100 + KEY_LED_PULSE_MS - 1));
    assert(!key_led_off(0, 100 + KEY_LED_PULSE_MS));
    assert(!key_led_off(0, 100)); /* A full timer wrap cannot start a new pulse. */
    assert(key_led_off(2, 65530));
    assert(key_led_off(0, (uint16_t)(65530u + KEY_LED_PULSE_MS - 1)));
    assert(!key_led_off(0, (uint16_t)(65530u + KEY_LED_PULSE_MS)));
    assert(key_led_off(4, 200));
    key_led_reset(); assert(!key_led_off(0, 201));
    assert(key_led_off(7, 300));
    assert(!key_led_off(0, 300 + KEY_LED_PULSE_MS));
    puts("PASS: physical-key LED pulse, 10 ms expiry, no phantom wrap, reset, simultaneous");
    return 0;
}
