/* Small EP0 commands avoid multi-packet OUT transfers and unbounded buffers. */
#include "transport.h"
#include "boot_command.h"
#include "settings.h"
#ifdef __SDCC
#pragma nooverlay
#endif
static uint8_t pending;
static uint16_t pending_index;

void KachiControlCancel(void) { pending = 0; }

uint8_t KachiControlSetup(void) {
    uint8_t length = UsbSetupBuf->wLengthL;
    uint8_t request = UsbSetupBuf->bRequest, i, key, offset;
    uint16_t index = ((uint16_t)UsbSetupBuf->wIndexH << 8) | UsbSetupBuf->wIndexL;
    uint8_t command = boot_command_decode((uint8_t *)UsbSetupBuf, kachi_waiting);
    if (command == BOOT_COMMAND_STATUS) {
        Ep0Buffer[0] = 'K'; Ep0Buffer[1] = 'B';
        Ep0Buffer[2] = 2; Ep0Buffer[3] = kachi_waiting;
        return 4;
    }
    if (command == BOOT_COMMAND_ENTER && !settings_saving) {
        pending = BOOT_ENTER_REQUEST;
        return 0;
    }
    if (UsbSetupBuf->wValueL != (BOOT_COMMAND_MAGIC & 0xff) ||
        UsbSetupBuf->wValueH != (BOOT_COMMAND_MAGIC >> 8) || UsbSetupBuf->wLengthH)
        return 0xff;
    if (UsbSetupBuf->bRequestType == 0xc0) {
        if (request == CONFIG_INFO_REQUEST && !index && length == 8) {
            Ep0Buffer[0] = 'K'; Ep0Buffer[1] = 'C'; Ep0Buffer[2] = 2;
            Ep0Buffer[3] = MAX_TEXT_LENGTH; Ep0Buffer[4] = MAX_REPEAT;
            Ep0Buffer[5] = settings_result; Ep0Buffer[6] = settings_saving;
            Ep0Buffer[7] = settings_dirty;
            return 8;
        }
        key = (uint8_t)index; offset = (uint8_t)(index >> 8);
        if (request == CONFIG_GET_REQUEST && key < KEY_COUNT &&
            offset < MACRO_BYTES && length &&
            length <= 8 &&
            length <= MACRO_BYTES - offset) {
            for (i = 0; i < length; ++i)
                Ep0Buffer[i] = settings_keys[key][offset + i];
            return length;
        }
    } else if (UsbSetupBuf->bRequestType == 0x40 && !length &&
               settings_can(request, index) && !kachi_boot_requested) {
        pending = request; pending_index = index;
        return 0;
    }
    return 0xff;
}

void KachiControlComplete(void) {
    if (pending == BOOT_ENTER_REQUEST) kachi_boot_requested = 1;
    else if (pending) settings_execute(pending, pending_index);
    pending = 0;
}
