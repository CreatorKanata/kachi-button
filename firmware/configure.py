"""Read/edit/save key macros through USB; text repeats with an interval in ms."""
import argparse
import json
import time

import config
from usb_control import UsbControl


def pack_macro(text, repeat, interval_ms):
    data = text.encode('ascii')
    if len(data) > config.MAX_TEXT_LENGTH or any(c < 32 or c > 126 for c in data):
        raise ValueError('Text must contain at most 32 printable US-layout ASCII characters')
    if not 1 <= repeat <= config.MAX_REPEAT:
        raise ValueError('Repeat must be 1..99')
    if not 0 <= interval_ms <= config.MAX_INTERVAL_MS:
        raise ValueError('Interval must be 0..60000 ms')
    return bytes((repeat, len(data), interval_ms & 255, interval_ms >> 8)) + data.ljust(config.MAX_TEXT_LENGTH, b'\0')


def read_info(usb):
    data = usb.transfer(0xc0, config.CONFIG_INFO_REQUEST, 8)
    if data[:5] != bytes((75, 67, 2, config.MAX_TEXT_LENGTH, config.MAX_REPEAT)):
        raise RuntimeError('Unsupported configuration protocol or limits')
    return {'result': data[5], 'saving': bool(data[6]), 'unsaved': bool(data[7])}


def read_key(usb, key):
    if not 0 <= key < len(config.PINS):
        raise ValueError('Key ID must be 0, 1, or 2')
    data = b''
    size = config.MAX_TEXT_LENGTH + 4
    for offset in range(0, size, 8):
        data += usb.transfer(0xc0, config.CONFIG_GET_REQUEST, min(8, size-offset), (offset << 8) | key)
    result = {'id': key, 'text': data[4:4+data[1]].decode('ascii'),
              'repeat': data[0], 'interval_ms': int.from_bytes(data[2:4], 'little')}
    if pack_macro(result['text'], result['repeat'], result['interval_ms']) != data:
        raise RuntimeError('Invalid configuration returned by device')
    return result


def set_key(usb, key, text, repeat, interval_ms):
    data = pack_macro(text, repeat, interval_ms)
    if not 0 <= key < len(config.PINS):
        raise ValueError('Key ID must be 0, 1, or 2')
    if read_info(usb)['saving']:
        raise RuntimeError('Device is saving; retry after completion')
    usb.transfer(0x40, config.CONFIG_BEGIN_REQUEST, index=key)
    for offset, value in enumerate(data):
        usb.transfer(0x40, config.CONFIG_BYTE_REQUEST, index=(value << 8) | offset)
    usb.transfer(0x40, config.CONFIG_APPLY_REQUEST)
    result = read_key(usb, key)
    if result != dict(id=key, text=text, repeat=repeat, interval_ms=interval_ms):
        raise RuntimeError('Configuration read-back mismatch')
    return result


def save(usb):
    usb.transfer(0x40, config.CONFIG_SAVE_REQUEST)
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline:
        info = read_info(usb)
        if not info['saving']:
            if info['result'] or info['unsaved']:
                raise RuntimeError('EEPROM save/read-back verification failed')
            return
        time.sleep(0.02)
    raise RuntimeError('Save completion timed out; inspect status before retrying')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--libusb')
    sub = parser.add_subparsers(dest='command', required=True)
    sub.add_parser('get')
    sub.add_parser('save')
    edit = sub.add_parser('set')
    edit.add_argument('key', type=int, choices=range(3))
    edit.add_argument('--text', required=True)
    edit.add_argument('--repeat', type=int, default=1)
    edit.add_argument('--interval-ms', type=int, default=0)
    edit.add_argument('--save', action='store_true')
    args = parser.parse_args()
    usb = UsbControl(args.libusb)
    try:
        usb.devices()
        read_info(usb)
        if args.command == 'set':
            set_key(usb, args.key, args.text, args.repeat, args.interval_ms)
        if args.command == 'save' or getattr(args, 'save', False):
            save(usb)
        print(json.dumps({'keys': [read_key(usb, i) for i in range(3)],
                          **read_info(usb)}, indent=2))
    finally:
        usb.close()


if __name__ == '__main__':
    main()
