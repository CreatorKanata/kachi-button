/* Settings edits apply after USB acknowledgement; saving is a foreground job. */
#include <string.h>
#include "settings.h"
#include "storage.h"
#ifdef __SDCC
#pragma nooverlay
#endif
uint8_t settings_keys[KEY_COUNT][MACRO_BYTES];
volatile uint8_t settings_saving, settings_result, settings_dirty;
static uint8_t staged[MACRO_BYTES], staged_key, received;
static const char defaults[KEY_COUNT][MAX_TEXT_LENGTH + 1] = KEY_TEXTS;

uint8_t settings_valid(uint8_t *macro) {
    uint8_t i;
    if (!macro[0] || macro[0] > MAX_REPEAT || macro[1] > MAX_TEXT_LENGTH)
        return 0;
    if (((uint16_t)macro[3] << 8 | macro[2]) > MAX_INTERVAL_MS) return 0;
    for (i = 0; i < MAX_TEXT_LENGTH; ++i) {
        if (i < macro[1]) {
            if (macro[4 + i] < 32 || macro[4 + i] > 126) return 0;
        } else if (macro[4 + i]) return 0;
    }
    return 1;
}

void settings_cancel_edit(void) { staged_key = KEY_COUNT; received = 0; }

void settings_init(void) {
    uint8_t key, pos;
    settings_cancel_edit();
    settings_saving = settings_result = settings_dirty = 0;
    if (storage_load()) return;
    memset(settings_keys, 0, sizeof settings_keys);
    for (key = 0; key < KEY_COUNT; ++key) {
        settings_keys[key][0] = 1;
        for (pos = 0; defaults[key][pos]; ++pos)
            settings_keys[key][4 + pos] = defaults[key][pos];
        settings_keys[key][1] = pos;
    }
}

uint8_t settings_can(uint8_t request, uint16_t index) {
    if (settings_saving) return 0;
    switch (request) {
    case CONFIG_BEGIN_REQUEST: return index < KEY_COUNT;
    case CONFIG_BYTE_REQUEST:
        return staged_key < KEY_COUNT && received < MACRO_BYTES &&
               (uint8_t)index == received;
    case CONFIG_APPLY_REQUEST:
        return !index && staged_key < KEY_COUNT && received == MACRO_BYTES &&
               settings_valid(staged);
    case CONFIG_SAVE_REQUEST: return !index;
    default: return 0;
    }
}

void settings_execute(uint8_t request, uint16_t index) {
    /* Called only from the USB ISR after settings_can and the status ACK. */
    switch (request) {
    case CONFIG_BEGIN_REQUEST:
        staged_key = (uint8_t)index;
        received = 0;
        break;
    case CONFIG_BYTE_REQUEST:
        staged[received++] = (uint8_t)(index >> 8);
        break;
    case CONFIG_APPLY_REQUEST:
        memcpy(settings_keys[staged_key], staged, MACRO_BYTES);
        settings_dirty = 1;
        settings_result = 0;
        settings_cancel_edit();
        break;
    case CONFIG_SAVE_REQUEST:
        settings_result = 1; /* Busy until write/read-back verification finishes. */
        settings_saving = 1;
        break;
    }
}

void settings_finish_save(uint8_t ok) {
    settings_result = ok ? 0 : 2;
    if (ok) settings_dirty = 0;
    /* Publish idle last so the host never observes an unfinished save as done. */
    settings_saving = 0;
}
