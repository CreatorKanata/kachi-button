# Short firmware upload commands; machine-specific paths stay in an ignored file.
.DEFAULT_GOAL := help
-include firmware/local.mk

PYTHON ?= python3
WCHISP ?= wchisp
LIBUSB ?= $(KACHI_LIBUSB)
FIRMWARE ?= firmware/build/compiled/kachi_button.ino.hex

.PHONY: help flash-first flash check-flash
help:
	@echo 'make flash-first  First flash or recovery: device must already be in WCH ISP'
	@echo 'make flash        Update a running Kachi Button (requests ISP automatically)'
	@echo 'Set tool paths in firmware/local.mk; neither command rebuilds the HEX.'

check-flash:
	@test -f "$(FIRMWARE)" || { echo 'HEX missing: build firmware or set FIRMWARE=/path/to/file.hex' >&2; exit 1; }
	@command -v "$(WCHISP)" >/dev/null 2>&1 || { echo 'wchisp missing: set WCHISP in firmware/local.mk' >&2; exit 1; }

# Native ISP has no Kachi application endpoint; use wchisp directly.
flash-first: check-flash
	"$(WCHISP)" flash "$(FIRMWARE)"

# Keep flash.py protocol, identity, single-target, and verification checks.
flash: check-flash
	KACHI_LIBUSB="$(LIBUSB)" "$(PYTHON)" firmware/flash.py "$(FIRMWARE)" --wchisp "$(WCHISP)"
