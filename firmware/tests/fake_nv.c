/* Native DataFlash double: byte-addressed storage with injected write failure. */
#include <assert.h>
#include <string.h>
#include "storage.h"
uint8_t fake_nv[EEPROM_CAPACITY];
int fail_after = -1, write_count;
void fake_reset(void) { memset(fake_nv, 0xff, sizeof fake_nv); fail_after = -1; write_count = 0; }
uint8_t nv_read(uint8_t address) { assert(address < EEPROM_CAPACITY); return fake_nv[address]; }
void nv_write(uint8_t address, uint8_t value) {
    assert(address < EEPROM_CAPACITY);
    if (fail_after >= 0 && write_count >= fail_after) return;
    ++write_count; fake_nv[address] = value;
}
