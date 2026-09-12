/* Test staged edits, persistent data, corruption and every interrupted write. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "storage.h"
extern uint8_t fake_nv[EEPROM_CAPACITY];
extern int fail_after, write_count;
void fake_reset(void);

static void command(uint8_t request, uint16_t index) {
    assert(settings_can(request, index)); settings_execute(request, index);
}
static void edit(uint8_t key, const char *text, uint8_t repeat, uint16_t interval) {
    uint8_t macro[MACRO_BYTES] = {0}, i;
    macro[0] = repeat; macro[1] = (uint8_t)strlen(text);
    macro[2] = (uint8_t)interval; macro[3] = (uint8_t)(interval >> 8);
    assert(macro[1] <= MAX_TEXT_LENGTH);
    memcpy(macro + 4, text, macro[1]);
    command(CONFIG_BEGIN_REQUEST, key);
    for (i = 0; i < MACRO_BYTES; ++i) command(CONFIG_BYTE_REQUEST, ((uint16_t)macro[i] << 8) | i);
    command(CONFIG_APPLY_REQUEST, 0);
}
int main(void) {
    uint8_t old[EEPROM_CAPACITY], saved[KEY_COUNT][MACRO_BYTES], malformed[MACRO_BYTES] = {1};
    int count, cut;
    fake_reset(); settings_init();
    assert(settings_keys[0][1] == 6 && !memcmp(settings_keys[0] + 4, "Go Go!", 6));
    assert(!settings_can(CONFIG_BYTE_REQUEST, 0));
    assert(!settings_can(CONFIG_BEGIN_REQUEST, KEY_COUNT));
    command(CONFIG_BEGIN_REQUEST, 1);
    assert(!settings_can(CONFIG_APPLY_REQUEST, 0));
    assert(!settings_can(CONFIG_BYTE_REQUEST, 1));
    settings_cancel_edit(); assert(!settings_can(CONFIG_BYTE_REQUEST, 0));
    malformed[0] = 0; assert(!settings_valid(malformed));
    malformed[0] = 1; malformed[1] = MAX_TEXT_LENGTH+1; assert(!settings_valid(malformed));
    malformed[1] = 1; malformed[4] = '\n'; assert(!settings_valid(malformed));
    malformed[4] = 'A'; malformed[2] = 255; malformed[3] = 255; assert(!settings_valid(malformed));
    edit(0, "Go!", 3, 250); edit(1, "", 1, 0);
    edit(2, "012345678901234567890123456789!~", MAX_REPEAT, MAX_INTERVAL_MS);
    memcpy(saved, settings_keys, sizeof saved);
    command(CONFIG_SAVE_REQUEST, 0);
    assert(settings_saving && !settings_can(CONFIG_BEGIN_REQUEST, 0));
    settings_finish_save(storage_save());
    assert(!settings_saving && !settings_result && !settings_dirty);
    settings_init(); assert(!memcmp(saved, settings_keys, sizeof saved));
    count = write_count; assert(storage_save()); assert(write_count == count);
    /* CRC detects changed payload, even when marker remains intact. */
    memcpy(old, fake_nv, sizeof old); fake_nv[10] ^= 1;
    settings_init(); assert(!memcmp(settings_keys[0]+4, "Go Go!", 6));
    memcpy(fake_nv, old, sizeof old); settings_init();
    edit(0, "Updated", 7, 1000); write_count = 0; assert(storage_save()); count = write_count;
    /* All partial commits recover either the intact prior record or defaults. */
    for (cut = 0; cut < count; ++cut) {
        memcpy(fake_nv, old, sizeof old); fail_after = -1; settings_init();
        edit(0, "Updated", 7, 1000); write_count = 0; fail_after = cut;
        assert(!storage_save()); fail_after = -1; settings_init();
        if (cut == 0) assert(!memcmp(settings_keys, saved, sizeof saved));
        else assert(!memcmp(settings_keys[0]+4, "Go Go!", 6));
    }
    puts("PASS: settings validation, atomic edits, persistence, idempotent save, corruption, all torn writes");
    return 0;
}
