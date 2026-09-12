"""Check that upload cannot detach an unarmed board or skip verification."""
from pathlib import Path
import tempfile
import unittest
from unittest.mock import Mock, patch

import config
from usb_control import require_libusb_version, require_identity
from flash import require_uploadable, upload


class FlashTests(unittest.TestCase):
    def test_manufacturer_migration_keeps_identity_checks(self):
        require_identity('Kachi Button', 'HAPT Lab, LLC')
        require_identity('Kachi Button', 'CreatorKanata')
        for product, manufacturer in [('Other', 'HAPT Lab, LLC'), ('Kachi Button', 'Other')]:
            with self.assertRaises(RuntimeError):
                require_identity(product, manufacturer)

    def test_reject_macos_hotplug_deadlock_dependency(self):
        with self.assertRaises(RuntimeError):
            require_libusb_version((1, 0, 29), 'darwin')
        require_libusb_version((1, 0, 30), 'darwin')
        require_libusb_version((1, 0, 29), 'linux')

    def test_reject_unknown_and_unarmed_status(self):
        for value in (b'', b'KB\x01\x00', b'KB\x03\x01', b'XX\x01\x01'):
            with self.assertRaises(RuntimeError):
                require_uploadable(value)
        for value in (b'KB\x01\x01', b'KB\x02\x00', b'KB\x02\x01'):
            require_uploadable(value)

    def test_upload_order_and_fail_closed(self):
        with tempfile.NamedTemporaryFile(suffix='.hex') as file:
            path = Path(file.name)
            usb, run = Mock(), Mock()
            usb.status.return_value = b'KB\x01\x00'
            with patch('flash.shutil.which', return_value='/bin/wchisp'):
                with self.assertRaises(RuntimeError):
                    upload(usb, path, 'wchisp', run)
                usb.enter.assert_not_called()
                run.assert_not_called()
                usb.status.return_value = b'KB\x02\x00'
                events = []
                usb.enter.side_effect = lambda: events.append('enter')
                usb.close.side_effect = lambda: events.append('close')
                run.side_effect = lambda *a, **kw: events.append('flash')
                upload(usb, path, 'wchisp', run)
                self.assertEqual(events, ['enter', 'close', 'flash'])
                run.assert_called_once_with(['/bin/wchisp', '--retry',
                    str(config.ISP_SCAN_SECONDS), 'flash', str(path)], check=True)

    def test_missing_tool_does_not_enter_isp(self):
        usb = Mock()
        with tempfile.NamedTemporaryFile() as file, patch('flash.shutil.which', return_value=None):
            with self.assertRaises(RuntimeError):
                upload(usb, Path(file.name), 'missing')
            usb.enter.assert_not_called()


if __name__ == '__main__':
    unittest.main()
