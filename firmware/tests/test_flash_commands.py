"""Exercise Make upload entry points with fake tools; never access physical USB."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

MAKEFILE = Path(__file__).resolve().parents[2] / 'Makefile'


@unittest.skipUnless(shutil.which('make'), 'make is required for command integration tests')
class FlashCommandTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='kachi commands ')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'firmware').mkdir()
        shutil.copy2(MAKEFILE, self.root / 'Makefile')
        self.log = self.root / 'calls.json'
        self.hex = self.root / 'firmware image.hex'
        self.hex.write_text(':00000001FF\n')
        self.python = self.root / 'python tool'
        self.wchisp = self.root / 'wchisp tool'
        for tool in (self.python, self.wchisp):
            tool.write_text(f'#!{sys.executable}\n'
                            'import json, os, sys\n'
                            'from pathlib import Path\n'
                            "Path(os.environ['KACHI_TEST_LOG']).write_text(json.dumps({"
                            "'args':sys.argv,'libusb':os.environ.get('KACHI_LIBUSB')}))\n"
                            "sys.exit(int(os.environ.get('KACHI_TEST_EXIT', '0')))\n")
            tool.chmod(0o755)
        self.library = self.root / 'lib usb.dylib'
        (self.root / 'firmware/local.mk').write_text(
            f'PYTHON = {self.python}\nWCHISP = {self.wchisp}\n'
            f'LIBUSB = {self.library}\nFIRMWARE = {self.hex}\n')

    def invoke(self, *args, failure=False):
        env = dict(os.environ, KACHI_TEST_LOG=str(self.log),
                   KACHI_TEST_EXIT='7' if failure else '0')
        return subprocess.run(['make', *args], cwd=self.root, env=env,
                              capture_output=True, text=True)

    def test_first_flash_bypasses_application_and_keeps_verification(self):
        result = self.invoke('flash-first')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(self.log.read_text())['args'],
                         [str(self.wchisp), 'flash', str(self.hex)])

    def test_update_uses_application_checks_and_library_with_spaced_paths(self):
        result = self.invoke('flash')
        self.assertEqual(result.returncode, 0, result.stderr)
        call = json.loads(self.log.read_text())
        self.assertEqual(call['args'], [str(self.python), 'firmware/flash.py',
                                       str(self.hex), '--wchisp', str(self.wchisp)])
        self.assertEqual(call['libusb'], str(self.library))

    def test_missing_hex_never_launches_tools(self):
        self.hex.unlink()
        for target in ('flash-first', 'flash'):
            result = self.invoke(target)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('HEX missing', result.stderr)
            self.assertFalse(self.log.exists())

    def test_missing_wchisp_never_starts_application_isp(self):
        self.wchisp.unlink()
        result = self.invoke('flash')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('wchisp missing', result.stderr)
        self.assertFalse(self.log.exists())

    def test_tool_failures_are_not_reported_as_success(self):
        for target in ('flash-first', 'flash'):
            self.assertNotEqual(self.invoke(target, failure=True).returncode, 0)

    def test_default_make_only_shows_help(self):
        self.assertEqual(self.invoke().returncode, 0)
        self.assertFalse(self.log.exists())

    def test_explicit_hex_overrides_local_config(self):
        alternate = self.root / 'another image.hex'
        alternate.write_text(':00000001FF\n')
        result = self.invoke('flash-first', f'FIRMWARE={alternate}')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(self.log.read_text())['args'][-1], str(alternate))


if __name__ == '__main__':
    unittest.main()
