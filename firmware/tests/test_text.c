/* Validate exact text reports, Caps Lock handling, retry and cancellation. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "text.h"
#include "settings.h"

static void expect_empty(uint8_t *r) {
    uint8_t i;
    for (i = 0; i < 8; ++i) assert(r[i] == 0);
}

int main(void) {
    /* Independent HID report expectations for Go Go!, Hi!, Thx. */
    const uint8_t codes[3][6] = {{0x0a, 0x12, 0x2c, 0x0a, 0x12, 0x1e},
                               {0x0b, 0x0c, 0x1e}, {0x17, 0x0b, 0x1b}};
    const uint8_t shifts[3][6] = {{2, 0, 0, 2, 0, 2}, {2, 0, 2}, {2, 0, 0}};
    const uint8_t lengths[] = {6, 3, 3};
    uint8_t r[8], retry[8], caps, id, pos, i;
    settings_init();
    for (caps = 0; caps < 2; ++caps) {
        for (id = 0; id < 3; ++id) {
            text_reset();
            assert(text_busy());
            assert(!text_start(id + 1));
            assert(text_peek(caps, r));
            expect_empty(r);
            text_accepted(0);
            assert(!text_busy());
            assert(!text_start(0) && !text_start(KEY_COUNT + 1));
            assert(text_start(id + 1));
            assert(!text_start((id + 1) % KEY_COUNT + 1));
            for (pos = 0; pos < lengths[id]; ++pos) {
                uint8_t expected_shift = shifts[id][pos];
                if (caps && codes[id][pos] >= 4 && codes[id][pos] <= 0x1d)
                    expected_shift ^= 2;
                assert(text_peek(caps, r));
                assert(r[0] == expected_shift && r[2] == codes[id][pos]);
                assert(r[1] == 0);
                for (i = 3; i < 8; ++i) assert(r[i] == 0);
                /* A busy endpoint does not advance the text cursor. */
                assert(text_peek(caps, retry));
                assert(memcmp(r, retry, 8) == 0);
                text_accepted(0);
                assert(text_peek(caps, r));
                expect_empty(r);
                text_accepted(0);
            }
            assert(!text_busy() && !text_peek(caps, r));
        }
    }
    assert(text_start(1));
    text_accepted(0); /* Simulate disconnect after key-down. */
    text_reset();
    assert(text_peek(0, r));
    expect_empty(r);
    text_accepted(0);
    assert(!text_busy());
    puts("PASS: exact Go Go!/Hi!/Thx reports, case, space, punctuation, releases, retry, busy, cancel");
    return 0;
}
