"""Stage pinned CH55xduino USB sources, run native tests, and build the CH552 HEX."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess

import config

ROOT = Path(__file__).resolve().parent


def replace_once(text, old, new):
    """Fail closed if the pinned upstream source no longer matches our patch."""
    if text.count(old) != 1:
        raise RuntimeError(f"Expected one upstream match: {old!r}")
    return text.replace(old, new, 1)


def stage(data, out):
    core = data / "packages/CH55xDuino/hardware/mcs51" / config.CORE_VERSION
    upstream = core / "libraries/Generic_Examples/examples/05.USB/HidKeyboard/src/userUsbHidKeyboard"
    sketch = out / "kachi_button"
    dst = sketch / "src/userUsbHidKeyboard"
    dst.mkdir(parents=True, exist_ok=True)
    shutil.copytree(ROOT / "src", sketch / "src", dirs_exist_ok=True)
    shutil.copy2(ROOT / "kachi_button.ino", sketch)
    (sketch / "src/config.h").write_text(config.header())
    for name in ("USBhandler.c", "USBhandler.h", "USBconstant.c", "USBconstant.h"):
        shutil.copy2(upstream / name, dst / name)

    handler = dst / "USBhandler.c"
    text = handler.read_text()
    text = replace_once(text, '#include "USBhandler.h"',
                        '#include "USBhandler.h"\nvoid KachiUsbReset(void);\n'
                        'void KachiControlCancel(void);\nuint8_t KachiControlSetup(void);\n'
                        'void KachiControlComplete(void);')
    text = replace_once(text, 'UsbConfig = UsbSetupBuf->wValueL;',
                        'KachiUsbReset();\n        UsbConfig = UsbSetupBuf->wValueL;')
    text = replace_once(text, '    UsbConfig = 0;',
                        '    UsbConfig = 0;\n    usbSuspended = 0;\n    KachiUsbReset();')
    text = replace_once(text, 'usbSuspended = 1;',
                        'KachiUsbReset();\n      usbSuspended = 1;')
    text = replace_once(text, '  __data uint8_t len = USB_RX_LEN;',
                        '  __data uint8_t len = USB_RX_LEN;\n  KachiControlCancel();')
    text = replace_once(text, """        switch (SetupReq) {
        default:
          len = 0xFF; // command not supported
          break;
        }""", '        len = KachiControlSetup();')
    text = replace_once(text, 'void USB_EP0_IN() {',
                        'void USB_EP0_IN() {\n  KachiControlComplete();')
    handler.write_text(text)

    descriptors = dst / "USBconstant.c"
    text = descriptors.read_text()
    text = replace_once(text, '0x1209', hex(config.USB_VID))
    text = replace_once(text, '0xc55D', hex(config.USB_PID))
    text = replace_once(text, '(USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_REMOTEWAKEUP)',
                        'USB_CONFIG_ATTR_RESERVED')
    text = replace_once(text, 'USB_CONFIG_POWER_MA(200)',
                        f'USB_CONFIG_POWER_MA({config.USB_POWER_MA})')
    for name, value in (("Product", config.USB_PRODUCT),
                        ("Manufacturer", config.USB_MANUFACTURER)):
        replacement = (f'__code uint16_t {name}Descriptor[] = {{\n'
                       f'    (({len(value) + 1} * 2) | (DTYPE_String << 8)),\n'
                       + ', '.join(str(ord(c)) for c in value) + '\n};')
        text, count = re.subn(rf'__code uint16_t {name}Descriptor\[\] = \{{.*?\}};',
                              replacement, text, flags=re.S)
        assert count == 1
    # No unique chip serial is implemented; avoid presenting a shared serial.
    text = replace_once(text, '.SerialNumStrIndex = 3', '.SerialNumStrIndex = 0')
    descriptors.write_text(text)
    return sketch


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data', type=Path, required=True, help='Arduino data directory')
    parser.add_argument('--out', type=Path, default=ROOT / 'build')
    parser.add_argument('--cli', default='arduino-cli')
    parser.add_argument('--test-only', action='store_true')
    args = parser.parse_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    sketch = stage(args.data.resolve(), out)
    suites = {
        'keys': ['src/keys.c'],
        'key_led': ['src/key_led.c'],
        'text': ['src/text.c', 'src/settings.c', 'src/storage.c', 'tests/fake_nv.c'],
        'repeat': ['src/text.c', 'src/settings.c', 'src/storage.c', 'tests/fake_nv.c'],
        'settings': ['src/settings.c', 'src/storage.c', 'tests/fake_nv.c'],
        'boot_gesture': ['src/boot_gesture.c'],
        'boot_command': ['src/boot_command.c'],
        'device_control': ['src/device_control.c', 'src/chip_id.c', 'src/boot_command.c',
                           'src/settings.c', 'src/storage.c', 'tests/fake_nv.c'],
    }
    for name, sources in suites.items():
        test = out / ('test_' + name)
        flags = ['-include', str(ROOT / 'tests/usb_stub.h')] if name == 'device_control' else []
        subprocess.run(['cc', '-std=c99', '-Wall', '-Wextra', '-Werror',
                        *flags, '-I' + str(sketch / 'src'), str(ROOT / f'tests/test_{name}.c'),
                        *[str(ROOT / source) for source in sources], '-o', str(test)], check=True)
        subprocess.run([str(test)], check=True)
    subprocess.run(['python3', '-m', 'unittest', 'discover', '-s',
                    str(ROOT / 'tests'), '-p', 'test_*.py'], check=True,
                   env=dict(os.environ, PYTHONPATH=str(ROOT)))
    if not args.test_only:
        env = dict(os.environ, ARDUINO_DIRECTORIES_DATA=str(args.data.resolve()),
                   ARDUINO_DIRECTORIES_USER=str(out / 'sketchbook'))
        subprocess.run([args.cli, 'compile', '--fqbn', config.FQBN,
                        '--build-path', str(out / 'compiled'), str(sketch)],
                       check=True, env=env)


if __name__ == '__main__':
    main()
