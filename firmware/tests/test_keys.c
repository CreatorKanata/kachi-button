/* Native tests exercise the same scanner and report builder as the firmware. */
#include <assert.h>
#include <stdio.h>
#include "keys.h"

int main(void) {
    uint8_t i;
    const uint8_t expected[] = {1, 2, 3};
    for (i = 0; i < KEY_COUNT; ++i) {
        uint8_t mask = 1 << i;
        keys_reset(0, 0);
        assert(keys_scan(0, DEBOUNCE_MS, 1) == 0);
        assert(keys_scan(mask, 100, 1) == 0);
        assert(keys_scan(mask, 100 + DEBOUNCE_MS - 1, 1) == 0);
        assert(keys_scan(mask, 100 + DEBOUNCE_MS, 1) == expected[i]);
        assert(keys_scan(mask, 500, 1) == 0); /* No hold repeat. */
        keys_scan(0, 600, 1);
        keys_scan(0, 600 + DEBOUNCE_MS, 1);
        keys_scan(mask, 700, 1);
        assert(keys_scan(mask, 700 + DEBOUNCE_MS, 1) == expected[i]);
    }

    keys_reset(1, 0); /* Startup-held key must first be released. */
    assert(keys_scan(1, 100, 1) == 0);
    keys_scan(0, 200, 1);
    keys_scan(0, 200 + DEBOUNCE_MS, 1);
    keys_scan(1, 300, 1);
    keys_scan(0, 301, 1); /* Bounce restarts stable interval. */
    keys_scan(1, 302, 1);
    assert(keys_scan(1, 302 + DEBOUNCE_MS - 1, 1) == 0);
    assert(keys_scan(1, 302 + DEBOUNCE_MS, 1) == 1);

    keys_reset(0, 0);
    keys_scan(0, DEBOUNCE_MS, 1);
    keys_scan(2, 100, 0);
    assert(keys_scan(2, 100 + DEBOUNCE_MS, 0) == 0);
    assert(keys_scan(2, 200, 1) == 0); /* Busy presses are consumed. */

    keys_reset(0, 65500);
    keys_scan(0, 65500 + DEBOUNCE_MS, 1);
    keys_scan(4, 65530, 1);
    assert(keys_scan(4, (uint16_t)(65530u + DEBOUNCE_MS), 1) == 3);

    keys_reset(0, 0);
    keys_scan(0, DEBOUNCE_MS, 1);
    keys_scan(7, 100, 1);
    assert(keys_scan(7, 100 + DEBOUNCE_MS, 1) == 1);
    assert(keys_scan(7, 200, 1) == 0);
    puts("PASS: mapping, debounce, held startup, repeat, busy, wrap, simultaneous");
    return 0;
}
