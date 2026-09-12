/* A validated host request can enter ISP from normal or write-wait mode. */
#include "boot_command.h"

uint8_t boot_command_decode(uint8_t *setup, uint8_t waiting) {
    (void)waiting; /* Retain the decoder API; protocol v2 permits either mode. */
    if (setup[2] != (BOOT_COMMAND_MAGIC & 0xff) ||
        setup[3] != (BOOT_COMMAND_MAGIC >> 8) ||
        setup[4] || setup[5] || setup[7])
        return BOOT_COMMAND_INVALID;
    if (setup[0] == 0xc0 && setup[1] == BOOT_STATUS_REQUEST && setup[6] == 4)
        return BOOT_COMMAND_STATUS;
    if (setup[0] == 0x40 && setup[1] == BOOT_ENTER_REQUEST && !setup[6])
        return BOOT_COMMAND_ENTER;
    return BOOT_COMMAND_INVALID;
}
