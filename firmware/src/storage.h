/* A versioned, CRC-checked record fits entirely in CH552's 128-byte DataFlash. */
#ifndef KACHI_STORAGE_H
#define KACHI_STORAGE_H
#include <stdint.h>
#include "config.h"
#define STORAGE_CRC_OFFSET (3 + KEY_COUNT * MACRO_BYTES)
#define STORAGE_MARK_OFFSET (STORAGE_CRC_OFFSET + 2)
#if STORAGE_MARK_OFFSET >= EEPROM_CAPACITY
#error "Settings record exceeds DataFlash"
#endif
uint8_t storage_load(void);
uint8_t storage_save(void);
uint8_t nv_read(uint8_t address);
void nv_write(uint8_t address, uint8_t value);
#endif
