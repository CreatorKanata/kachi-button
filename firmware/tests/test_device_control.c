/* Exercise the actual EP0 adapter, shared buffer aliasing, and ACK-only writes. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "boot_command.h"
uint8_t Ep0Buffer[8];
volatile uint8_t kachi_waiting, kachi_boot_requested;
void fake_reset(void);

static uint8_t setup(uint8_t type, uint8_t request, uint16_t index, uint8_t length) {
    KachiControlCancel();
    Ep0Buffer[0] = type; Ep0Buffer[1] = request;
    Ep0Buffer[2] = BOOT_COMMAND_MAGIC & 255; Ep0Buffer[3] = BOOT_COMMAND_MAGIC >> 8;
    Ep0Buffer[4] = (uint8_t)index; Ep0Buffer[5] = (uint8_t)(index >> 8);
    Ep0Buffer[6] = length; Ep0Buffer[7] = 0;
    return KachiControlSetup();
}
int main(void) {
    uint8_t macro[MACRO_BYTES] = {3, 3, 250, 0, 'G','o','!'}, i;
    fake_reset(); settings_init();
    assert(setup(0xc0, CONFIG_INFO_REQUEST,0,8) == 8);
    assert(Ep0Buffer[0]=='K' && Ep0Buffer[1]=='C' && Ep0Buffer[2]==2);
    assert(setup(0x40,BOOT_ENTER_REQUEST,0,0)==0 && !kachi_boot_requested);
    /* Unrelated SETUP before status ACK cancels pending boot entry. */
    assert(setup(0xc0,BOOT_STATUS_REQUEST,0,4)==4);
    KachiControlComplete(); assert(!kachi_boot_requested);
    assert(setup(0x40,BOOT_ENTER_REQUEST,0,0)==0);
    KachiControlComplete(); assert(kachi_boot_requested); kachi_boot_requested=0;
    assert(setup(0x40,CONFIG_BEGIN_REQUEST,0,0)==0); KachiControlComplete();
    for (i=0;i<MACRO_BYTES;++i) {
        assert(setup(0x40,CONFIG_BYTE_REQUEST,((uint16_t)macro[i]<<8)|i,0)==0);
        KachiControlComplete();
    }
    assert(setup(0x40,CONFIG_APPLY_REQUEST,0,0)==0);
    assert(settings_keys[0][0]==1); KachiControlComplete();
    assert(!memcmp(settings_keys[0],macro,MACRO_BYTES));
    /* Writing response byte 6 must not change the loop's cached wLength. */
    assert(setup(0xc0,CONFIG_GET_REQUEST,0,8)==8);
    assert(!memcmp(Ep0Buffer,macro,8));
    assert(setup(0xc0,CONFIG_GET_REQUEST,32u<<8,4)==4);
    assert(!memcmp(Ep0Buffer,macro+32,4));
    assert(setup(0xc0,CONFIG_GET_REQUEST,32u<<8,8)==0xff);
    assert(setup(0xc0,CONFIG_GET_REQUEST,KEY_COUNT,8)==0xff);
    assert(setup(0xc0,CONFIG_GET_REQUEST,0,9)==0xff);
    assert(setup(0x40,CONFIG_BEGIN_REQUEST,0,1)==0xff);
    settings_saving=1;
    assert(setup(0x40,BOOT_ENTER_REQUEST,0,0)==0xff);
    assert(setup(0x40,CONFIG_BEGIN_REQUEST,0,0)==0xff);
    assert(setup(0xc0,CONFIG_INFO_REQUEST,0,8)==8 && Ep0Buffer[6]==1);
    puts("PASS: EP0 shared buffer, ACK gating, remote ISP, config packets, save exclusion");
    return 0;
}
