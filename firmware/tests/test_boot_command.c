/* Reject malformed USB requests and accept remote entry from either mode. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "boot_command.h"

int main(void) {
    uint8_t enter[8] = {0x40, BOOT_ENTER_REQUEST, BOOT_COMMAND_MAGIC & 0xff,
                       BOOT_COMMAND_MAGIC >> 8, 0, 0, 0, 0};
    uint8_t status[8] = {0xc0, BOOT_STATUS_REQUEST, BOOT_COMMAND_MAGIC & 0xff,
                        BOOT_COMMAND_MAGIC >> 8, 0, 0, 4, 0};
    uint8_t bad[8], i;
    assert(boot_command_decode(enter, 0) == BOOT_COMMAND_ENTER);
    assert(boot_command_decode(enter, 1) == BOOT_COMMAND_ENTER);
    assert(boot_command_decode(status, 0) == BOOT_COMMAND_STATUS);
    assert(boot_command_decode(status, 1) == BOOT_COMMAND_STATUS);
    for (i = 0; i < 8; ++i) {
        memcpy(bad, enter, sizeof bad);
        bad[i] ^= 1;
        assert(boot_command_decode(bad, 1) == BOOT_COMMAND_INVALID);
        memcpy(bad, status, sizeof bad);
        bad[i] ^= 1;
        assert(boot_command_decode(bad, 1) == BOOT_COMMAND_INVALID);
    }
    puts("PASS: vendor request validation and normal and waiting remote entry");
    return 0;
}
