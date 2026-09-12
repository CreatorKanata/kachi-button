/* Invalidate first, verify each byte, commit last; torn records load defaults. */
#include "settings.h"
#include "storage.h"

static uint16_t crc_byte(uint16_t crc, uint8_t value) {
    uint8_t bit;
    crc ^= (uint16_t)value << 8;
    for (bit = 0; bit < 8; ++bit)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    return crc;
}

static uint8_t content(uint8_t address) {
    if (address == 0) return 'K';
    if (address == 1) return 'C';
    if (address == 2) return SETTINGS_SCHEMA;
    return ((uint8_t *)settings_keys)[address - 3];
}

uint8_t storage_load(void) {
    uint8_t i, value;
    uint16_t crc = 0xffff;
    if (nv_read(STORAGE_MARK_OFFSET) != SETTINGS_COMMIT_MARK ||
        nv_read(0) != 'K' || nv_read(1) != 'C' || nv_read(2) != SETTINGS_SCHEMA)
        return 0;
    for (i = 0; i < STORAGE_CRC_OFFSET; ++i) {
        value = nv_read(i);
        crc = crc_byte(crc, value);
        if (i >= 3) ((uint8_t *)settings_keys)[i - 3] = value;
    }
    if (nv_read(STORAGE_CRC_OFFSET) != (uint8_t)crc ||
        nv_read(STORAGE_CRC_OFFSET + 1) != (uint8_t)(crc >> 8)) return 0;
    for (i = 0; i < KEY_COUNT; ++i)
        if (!settings_valid(settings_keys[i])) return 0;
    return 1;
}

static uint8_t write_checked(uint8_t address, uint8_t value) {
    if (nv_read(address) != value) nv_write(address, value);
    return nv_read(address) == value;
}

uint8_t storage_save(void) {
    uint8_t i, value, same = 1;
    uint16_t crc = 0xffff;
    for (i = 0; i < STORAGE_CRC_OFFSET; ++i) {
        value = content(i);
        crc = crc_byte(crc, value);
        if (nv_read(i) != value) same = 0;
    }
    if (same && nv_read(STORAGE_CRC_OFFSET) == (uint8_t)crc &&
        nv_read(STORAGE_CRC_OFFSET + 1) == (uint8_t)(crc >> 8) &&
        nv_read(STORAGE_MARK_OFFSET) == SETTINGS_COMMIT_MARK) return 1;
    if (!write_checked(STORAGE_MARK_OFFSET, 0)) return 0;
    for (i = 0; i < STORAGE_CRC_OFFSET; ++i)
        if (!write_checked(i, content(i))) return 0;
    if (!write_checked(STORAGE_CRC_OFFSET, (uint8_t)crc) ||
        !write_checked(STORAGE_CRC_OFFSET + 1, (uint8_t)(crc >> 8))) return 0;
    return write_checked(STORAGE_MARK_OFFSET, SETTINGS_COMMIT_MARK);
}
