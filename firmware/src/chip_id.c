/* CH552 datasheet 6.7: code-memory ID bytes in most-significant-first order. */
#include "config.h"
#include "chip_id.h"
#ifdef __SDCC
#pragma nooverlay
#define READ_CODE(address) (*(__code uint8_t *)(address))
#else
/* Native tests substitute code memory, including the reserved byte. */
extern uint8_t chip_id_read_code(uint16_t address);
#define READ_CODE(address) chip_id_read_code(address)
#endif
void chip_id_read(uint8_t *output) {
    static const uint16_t addresses[5] = CHIP_UID_ADDRESSES;
    uint8_t i;
    for (i = 0; i < 5; ++i) output[i] = READ_CODE(addresses[i]);
}
