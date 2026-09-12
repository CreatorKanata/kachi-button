/* Validate the small vendor-control protocol before allowing native ISP. */
#ifndef KACHI_BOOT_COMMAND_H
#define KACHI_BOOT_COMMAND_H
#include <stdint.h>
#include "config.h"
enum { BOOT_COMMAND_INVALID, BOOT_COMMAND_STATUS, BOOT_COMMAND_ENTER };
uint8_t boot_command_decode(uint8_t *setup, uint8_t waiting);
#endif
