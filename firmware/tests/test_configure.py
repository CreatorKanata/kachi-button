"""Validate encoding boundaries and exact USB commands for a ms-timed macro."""
import unittest
from unittest.mock import Mock, patch
import config
from configure import pack_macro, set_key

class ConfigureTests(unittest.TestCase):
    def test_bounds(self):
        for args in [('a'*33,1,0), ('\n',1,0), ('ok',0,0), ('ok',100,0), ('ok',1,-1), ('ok',1,60001)]:
            with self.assertRaises(ValueError): pack_macro(*args)
        with self.assertRaises(UnicodeEncodeError): pack_macro('あ',1,0)
        self.assertEqual(pack_macro('Go!',3,250)[:7], b'\x03\x03\xfa\x00Go!')
        self.assertEqual(len(pack_macro('x'*32,99,60000)),36)

    def test_wire_sequence(self):
        usb = Mock()
        expected = dict(id=2,text='Go!',repeat=3,interval_ms=250)
        with patch('configure.read_info', return_value={'saving':False}), patch('configure.read_key', return_value=expected):
            self.assertEqual(set_key(usb,2,'Go!',3,250),expected)
        calls = usb.transfer.call_args_list
        self.assertEqual(len(calls),38)
        self.assertEqual(calls[0].args,(0x40,config.CONFIG_BEGIN_REQUEST))
        self.assertEqual(calls[0].kwargs,dict(index=2))
        self.assertEqual(calls[3].kwargs,dict(index=(250<<8)|2))
        self.assertEqual(calls[-1].args,(0x40,config.CONFIG_APPLY_REQUEST))

if __name__ == '__main__': unittest.main()
