/* Snapshot each macro; repeat with nonblocking gaps and complete key releases. */
#include <string.h>
#include "text.h"
#include "settings.h"

/* US-layout mapping derived from CH55xduino USBHIDKeyboard.c (LGPL-2.1). */
static const uint8_t ascii_map[95] = {
    0x2c,0x9e,0xb4,0xa0,0xa1,0xa2,0xa4,0x34,0xa6,0xa7,0xa5,0xae,0x36,0x2d,0x37,0x38,
    0x27,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0xb3,0x33,0xb6,0x2e,0xb7,0xb8,
    0x9f,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x90,0x91,0x92,
    0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x2f,0x31,0x30,0xa3,0xad,
    0x35,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,
    0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0xaf,0xb1,0xb0,0xb5
};
enum { IDLE, KEY_DOWN, KEY_UP, GAP };
static uint8_t phase, position, remaining, length;
static uint8_t message[MAX_TEXT_LENGTH];
static uint16_t interval_ms, gap_started;

void text_reset(void) {
    remaining = position = length = 0;
    phase = KEY_UP;
}

uint8_t text_busy(void) { return phase != IDLE; }

uint8_t text_start(uint8_t action) {
    uint8_t *macro;
    if (text_busy() || !action || action > KEY_COUNT) return 0;
    macro = settings_keys[action - 1];
    if (!macro[1]) return 0;
    remaining = macro[0]; length = macro[1];
    interval_ms = (uint16_t)macro[2] | ((uint16_t)macro[3] << 8);
    memcpy(message, macro + 4, length);
    position = 0; phase = KEY_DOWN;
    return 1;
}

void text_tick(uint16_t now) {
    if (phase == GAP && (uint16_t)(now - gap_started) >= interval_ms)
        phase = KEY_DOWN;
}

uint8_t text_peek(uint8_t caps_lock, uint8_t *report) {
    uint8_t c, code, shift;
    memset(report, 0, 8);
    if (phase == IDLE || phase == GAP) return 0;
    if (phase == KEY_UP) return 1;
    c = message[position]; code = ascii_map[c - 32]; shift = !!(code & 0x80);
    if (caps_lock && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
        shift = !shift;
    report[0] = shift ? HID_LEFT_SHIFT : 0;
    report[2] = code & 0x7f;
    return 1;
}

void text_accepted(uint16_t now) {
    if (phase == KEY_DOWN) phase = KEY_UP;
    else if (phase == KEY_UP) {
        if (!remaining) phase = IDLE;
        else if (++position < length) phase = KEY_DOWN;
        else if (--remaining) {
            position = 0; gap_started = now; phase = GAP;
        } else phase = IDLE;
    }
}
