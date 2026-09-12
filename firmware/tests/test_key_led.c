/* Verify HID-entry duration, normally-off idle, key pulses and lifecycle wrap. */
#include <assert.h>
#include <stdio.h>
#include "key_led.h"
int main(void) {
    key_led_reset();
    assert(key_led_on(0, 100));
    assert(key_led_on(1, 500)); /* A press during entry does not extend 2 s. */
    assert(key_led_on(0, 100 + HID_START_LED_MS - 1));
    assert(!key_led_on(0, 100 + HID_START_LED_MS));
    assert(!key_led_on(0, 100)); /* No phantom flash on the next timer wrap. */
    assert(key_led_on(1, 3000));
    assert(key_led_on(0, 3000 + KEY_LED_PULSE_MS - 1));
    assert(!key_led_on(0, 3000 + KEY_LED_PULSE_MS));
    assert(!key_led_on(0, 3000));
    assert(key_led_on(2, 65530));
    assert(key_led_on(0, (uint16_t)(65530u + KEY_LED_PULSE_MS - 1)));
    assert(!key_led_on(0, (uint16_t)(65530u + KEY_LED_PULSE_MS)));
    key_led_reset(); /* Resume/new normal-mode session starts another indication. */
    assert(key_led_on(0, 65000));
    assert(key_led_on(0, (uint16_t)(65000u + HID_START_LED_MS - 1)));
    assert(!key_led_on(0, (uint16_t)(65000u + HID_START_LED_MS)));
    assert(key_led_on(7, 4000));
    assert(!key_led_on(0, 4000 + KEY_LED_PULSE_MS));
    puts("PASS: HID entry 2000 ms, idle off, key-on 30 ms, priority, reset, timer wrap");
    return 0;
}
