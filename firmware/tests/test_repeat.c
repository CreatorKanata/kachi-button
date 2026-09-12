/* Count actual HID presses/releases and test the ms gap across timer wrap. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "text.h"
#include "settings.h"
void fake_reset(void);

int main(void) {
    uint8_t report[8], i, repeat;
    uint16_t now = 65000, interval;
    fake_reset(); settings_init();
    for (interval = 0; ; interval = MAX_INTERVAL_MS) {
        memset(settings_keys[0], 0, MACRO_BYTES);
        settings_keys[0][0] = 3; settings_keys[0][1] = 2;
        settings_keys[0][2] = (uint8_t)interval; settings_keys[0][3] = (uint8_t)(interval >> 8);
        memcpy(settings_keys[0]+4, "A1", 2);
        text_reset(); text_accepted(now); assert(text_start(1));
        /* Editing settings cannot alter the active snapshot. */
        settings_keys[0][0] = 1; settings_keys[0][4] = 'Z';
        for (repeat = 0; repeat < 3; ++repeat) {
            for (i = 0; i < 2; ++i) {
                assert(text_peek(0, report));
                assert(report[2] == (i ? 0x1e : 0x04));
                text_accepted(now);
                assert(text_peek(0, report));
                assert(!report[0] && !report[2]); text_accepted(now);
            }
            if (repeat < 2) {
                assert(text_busy() && !text_peek(0, report));
                if (interval) {
                    text_tick((uint16_t)(now + interval - 1));
                    assert(!text_peek(0, report));
                }
                now = (uint16_t)(now + interval); text_tick(now);
                assert(text_peek(0, report));
            }
        }
        assert(!text_busy());
        if (interval == MAX_INTERVAL_MS) break;
    }
    /* Maximum repeat count is finite; reset while waiting discards the rest. */
    settings_keys[0][0] = MAX_REPEAT; settings_keys[0][1] = 1;
    settings_keys[0][2] = settings_keys[0][3] = 0;
    assert(text_start(1));
    for (repeat = 0; repeat < MAX_REPEAT; ++repeat) {
        text_tick(now); assert(text_peek(0, report)); text_accepted(now);
        assert(text_peek(0, report)); text_accepted(now);
    }
    assert(!text_busy());
    assert(text_start(1)); text_accepted(now); text_accepted(now);
    text_reset(); assert(text_peek(0, report) && !report[2]); text_accepted(now);
    assert(!text_busy());
    puts("PASS: repetitions, max count, zero/60000-ms gap, timer wrap, snapshot, cancellation");
    return 0;
}
