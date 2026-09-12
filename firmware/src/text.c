/* Fixed text macros: preserve case and release keys between every character. */
#include "text.h"

static const char messages[KEY_COUNT][TEXT_CAPACITY] = KEY_TEXTS;
enum { IDLE, KEY_DOWN, KEY_UP };
static uint8_t phase;
static uint8_t active;
static uint8_t position;

void text_reset(void) {
    active = 0;
    position = 0;
    phase = KEY_UP; /* Cancel the old macro and first send an empty report. */
}

uint8_t text_busy(void) { return phase != IDLE; }

uint8_t text_start(uint8_t action) {
    if (text_busy() || action == 0 || action > KEY_COUNT)
        return 0;
    active = action;
    position = 0;
    phase = KEY_DOWN;
    return 1;
}

uint8_t text_peek(uint8_t caps_lock, uint8_t *report) {
    uint8_t i, shift = 0;
    char c;
    for (i = 0; i < 8; ++i)
        report[i] = 0;
    if (phase == IDLE)
        return 0;
    if (phase == KEY_UP)
        return 1;
    c = messages[active - 1][position];
    if (c >= 'A' && c <= 'Z') {
        report[2] = c - 'A' + 4;
        shift = !caps_lock;
    } else if (c >= 'a' && c <= 'z') {
        report[2] = c - 'a' + 4;
        shift = !!caps_lock;
    } else if (c == ' ') {
        report[2] = 0x2c;
    } else if (c == '!') {
        report[2] = 0x1e;
        shift = 1;
    }
    /* config.py rejects unsupported characters before compilation. */
    report[0] = shift ? HID_LEFT_SHIFT : 0;
    return 1;
}

void text_accepted(void) {
    if (phase == KEY_DOWN) {
        phase = KEY_UP;
    } else if (phase == KEY_UP) {
        if (active && messages[active - 1][++position]) {
            phase = KEY_DOWN;
        } else {
            phase = IDLE;
            active = 0;
        }
    }
}
