"""Start native ISP only when the user is ready, then invoke verified wchisp flashing."""
import argparse
from pathlib import Path
import shutil
import subprocess

import config
from usb_control import UsbControl


def require_uploadable(status):
    if status not in (b'KB\x01\x01', b'KB\x02\x00', b'KB\x02\x01'):
        raise RuntimeError('Unsupported state. Legacy firmware requires the three-key startup gesture.')


def upload(usb, firmware, wchisp, run=subprocess.run):
    """Validate firmware capability and verify tool availability before detaching USB."""
    if not firmware.is_file():
        raise RuntimeError(f'Firmware file not found: {firmware}')
    tool = shutil.which(str(wchisp))
    if not tool:
        raise RuntimeError(f'wchisp not found: {wchisp}')
    require_uploadable(usb.status())
    usb.enter()
    usb.close()
    # wchisp retries discovery while CH552 changes USB identity; verification stays on.
    run([tool, '--retry', str(config.ISP_SCAN_SECONDS), 'flash', str(firmware)], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('firmware', nargs='?', type=Path)
    parser.add_argument('--enter', action='store_true', help='Enter native ISP without flashing')
    parser.add_argument('--status', action='store_true', help='Read state without starting ISP')
    parser.add_argument('--wchisp', default='wchisp')
    parser.add_argument('--libusb', help='Optional libusb shared-library path')
    args = parser.parse_args()
    if not args.status and not args.enter and not args.firmware:
        parser.error('provide a firmware HEX, --status, or --enter')
    usb = UsbControl(args.libusb)
    try:
        usb.devices()
        if args.status:
            status = usb.status()
            if status not in (b'KB\x01\x00', b'KB\x01\x01', b'KB\x02\x00', b'KB\x02\x01'):
                raise RuntimeError(f'Unknown firmware status: {status!r}')
            print('WRITE_WAIT (no timeout)' if status[-1] else 'NORMAL / HOLDING')
        elif args.enter:
            require_uploadable(usb.status())
            usb.enter()
            print('Native ISP requested; its own timeout now applies.')
        else:
            upload(usb, args.firmware.resolve(), args.wchisp)
    finally:
        usb.close()


if __name__ == '__main__':
    main()
