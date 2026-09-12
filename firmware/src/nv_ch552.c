/* Use pinned CH55xduino DataFlash API with protected per-byte register access. */
#include <Arduino.h>
#include "storage.h"

uint8_t nv_read(uint8_t address) {
    uint8_t saved = EA, value;
    EA = 0;
    value = eeprom_read_byte(address);
    EA = saved;
    return value;
}

void nv_write(uint8_t address, uint8_t value) {
    uint8_t saved = EA;
    EA = 0;
    eeprom_write_byte(address, value);
    EA = saved;
}
