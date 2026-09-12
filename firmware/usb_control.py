"""Minimal libusb EP0 access; leave the macOS keyboard driver attached."""
import ctypes as C
from ctypes.util import find_library

import config


class Descriptor(C.Structure):
    _fields_ = [(name, typ) for name, typ in (
        ('length', C.c_uint8), ('type', C.c_uint8), ('usb', C.c_uint16),
        ('klass', C.c_uint8), ('subclass', C.c_uint8), ('protocol', C.c_uint8),
        ('packet', C.c_uint8), ('vid', C.c_uint16), ('pid', C.c_uint16),
        ('version', C.c_uint16), ('manufacturer', C.c_uint8),
        ('product', C.c_uint8), ('serial', C.c_uint8), ('configs', C.c_uint8))]


class UsbControl:
    def __init__(self, library=None):
        path = library or find_library('usb-1.0')
        if not path:
            raise RuntimeError('libusb-1.0 not found; use --libusb /path/to/library')
        self.lib = C.CDLL(path)
        self.ctx = C.c_void_p()
        self.handle = C.c_void_p()
        signatures = {
            'libusb_init': ([C.POINTER(C.c_void_p)], C.c_int),
            'libusb_exit': ([C.c_void_p], None),
            'libusb_get_device_list': ([C.c_void_p, C.POINTER(C.POINTER(C.c_void_p))], C.c_ssize_t),
            'libusb_free_device_list': ([C.POINTER(C.c_void_p), C.c_int], None),
            'libusb_get_device_descriptor': ([C.c_void_p, C.POINTER(Descriptor)], C.c_int),
            'libusb_open': ([C.c_void_p, C.POINTER(C.c_void_p)], C.c_int),
            'libusb_close': ([C.c_void_p], None),
            'libusb_get_string_descriptor_ascii': ([C.c_void_p, C.c_uint8, C.c_void_p, C.c_int], C.c_int),
            'libusb_control_transfer': ([C.c_void_p, C.c_uint8, C.c_uint8,
                C.c_uint16, C.c_uint16, C.c_void_p, C.c_uint16, C.c_uint], C.c_int),
        }
        for name, (args, result) in signatures.items():
            fn = getattr(self.lib, name)
            fn.argtypes, fn.restype = args, result
        self.check(self.lib.libusb_init(C.byref(self.ctx)))

    @staticmethod
    def check(result):
        if result < 0:
            raise RuntimeError(f'libusb error {result}')
        return result

    def devices(self):
        """Collect descriptors while retaining the list until open completes."""
        devices = C.POINTER(C.c_void_p)()
        count = self.check(self.lib.libusb_get_device_list(self.ctx, C.byref(devices)))
        matches, isp = [], 0
        try:
            for i in range(count):
                desc = Descriptor()
                if self.lib.libusb_get_device_descriptor(devices[i], C.byref(desc)) < 0:
                    continue
                if desc.pid == 0x55e0 and desc.vid in (0x4348, 0x1a86):
                    isp += 1
                if (desc.vid, desc.pid) == (config.USB_VID, config.USB_PID):
                    matches.append((devices[i], desc))
            if len(matches) != 1 or isp:
                raise RuntimeError(f'Expected one Kachi target and no ISP targets; found {len(matches)} / {isp}')
            device, desc = matches[0]
            self.check(self.lib.libusb_open(device, C.byref(self.handle)))
            for index, expected in ((desc.product, config.USB_PRODUCT),
                                    (desc.manufacturer, config.USB_MANUFACTURER)):
                buf = C.create_string_buffer(256)
                n = self.check(self.lib.libusb_get_string_descriptor_ascii(self.handle, index, buf, len(buf)))
                if buf.raw[:n].decode('ascii') != expected:
                    raise RuntimeError('USB identity does not match Kachi Button')
        finally:
            self.lib.libusb_free_device_list(devices, 1)

    def transfer(self, direction, request, length=0):
        buf = C.create_string_buffer(max(length, 1))
        n = self.check(self.lib.libusb_control_transfer(self.handle, direction,
            request, config.BOOT_COMMAND_MAGIC, 0, buf, length,
            config.USB_CONTROL_TIMEOUT_MS))
        if n != length:
            raise RuntimeError(f'Incomplete USB transfer: {n} of {length} bytes')
        return buf.raw[:n]

    def status(self):
        return self.transfer(0xc0, config.BOOT_STATUS_REQUEST, 4)

    def enter(self):
        self.transfer(0x40, config.BOOT_ENTER_REQUEST)

    def close(self):
        if self.handle:
            self.lib.libusb_close(self.handle)
            self.handle = C.c_void_p()
        if self.ctx:
            self.lib.libusb_exit(self.ctx)
            self.ctx = C.c_void_p()
