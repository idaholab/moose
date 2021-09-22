#!/usr/bin/env python3
import sys
import unittest
import mock
import chigger

class TestChiggerObjectBase(unittest.TestCase):
    def testInitAndName(self):
        obj = chigger.base.ChiggerObject()
        self.assertEqual(obj.name(), obj.__class__.__name__)

        obj = chigger.base.ChiggerObject(name='foo')
        self.assertEqual(obj.name(), 'foo')

        sys.argv.append("ChiggerObject:name='test'")
        with self.assertLogs(level='INFO') as log:
            obj = chigger.base.ChiggerObject()
        self.assertEqual(len(log.output), 1)
        self.assertIn("Setting Option from Command Line: ChiggerObject:name='test'", log.output[0])
        self.assertEqual(obj.name(), 'test')

        sys.argv[-1] = "ChiggerObject:foo:name='test2'"
        with self.assertLogs(level='INFO') as log:
            obj = chigger.base.ChiggerObject(name='foo')
        self.assertEqual(len(log.output), 1)
        self.assertIn("(foo): Setting Option from Command Line: ChiggerObject:foo:name='test2'", log.output[0])
        self.assertEqual(obj.name(), 'test2')

    def testLogs(self):
        msg = "This is a test: {}"
        obj = chigger.base.ChiggerObject()

        with self.assertLogs(level='INFO') as log:
            obj.info(msg, 'INFO')
        self.assertEqual(len(log.output), 1)
        self.assertIn(msg.format('INFO'), log.output[0])

        with self.assertLogs(level='WARNING') as log:
            obj.warning(msg, 'WARNING')
        self.assertEqual(len(log.output), 1)
        self.assertIn(msg.format('WARNING'), log.output[0])

        with self.assertLogs(level='ERROR') as log:
            obj.error(msg, 'ERROR')
        self.assertEqual(len(log.output), 1)
        self.assertIn(msg.format('ERROR'), log.output[0])

        with self.assertLogs(level='DEBUG') as log:
            obj.debug(msg, 'DEBUG')
        self.assertEqual(len(log.output), 1)
        self.assertIn(msg.format('DEBUG'), log.output[0])

        with self.assertLogs(level='DEBUG') as log:
            obj.debug(msg, 'DEBUG', stack_info=True)
        self.assertEqual(len(log.output), 1)
        self.assertIn('Stack (most recent call last):', log.output[0])

    def testIsOptionValid(self):
        obj = chigger.base.ChiggerObject()
        self.assertFalse(obj.isParamValid('name'))
        obj.setParam('name', 'foo')
        self.assertTrue(obj.isParamValid('name'))

    def testIsOptionValid(self):
        obj = chigger.base.ChiggerObject()
        obj.parameters().add('foo', default='bar')

        self.assertTrue(obj.isParamDefault('foo'))
        obj.setParam('foo', 'not default')
        self.assertFalse(obj.isParamDefault('foo'))

    def testGetOption(self):
        obj = chigger.base.ChiggerObject(name='name')
        self.assertEqual(obj.getParam('name'), 'name')

        with self.assertLogs(level='WARNING') as log:
            obj.getParam('wrong')
        self.assertEqual(len(log.output), 1)
        self.assertIn("The parameter 'wrong' does not exist", log.output[0])

    def testSetOption(self):
        obj = chigger.base.ChiggerObject()
        obj.setParam('name', 'this')
        self.assertEqual(obj.getParam('name'), 'this')

        with self.assertLogs(level='WARNING') as log:
            obj.setParam('wrong', 42)
        self.assertEqual(len(log.output), 1)
        self.assertIn("The parameter 'wrong' does not exist", log.output[0])

    def testSetParams(self):
        obj = chigger.base.ChiggerObject()
        obj.setParams(name='this')
        self.assertEqual(obj.getParam('name'), 'this')

        with self.assertLogs(level='WARNING') as log:
            obj.setParams(wrong=42)
        self.assertEqual(len(log.output), 1)
        self.assertIn("The parameter 'wrong' does not exist", log.output[0])

    def assignParam(self):
        name = None
        def func(value):
            name = value
        obj = chigger.base.ChiggerObject(name='andrew')
        obj.assignParam('andrew', func)
        self.assertEqual(name, 'andrew')

        with self.assertLogs(level='WARNING') as log:
            obj.assignParam('wrong', func)
        self.assertEqual(len(log.output), 1)
        self.assertIn("The parameter 'wrong' does not exist", log.output[0])

    @mock.patch('builtins.print')
    def testPrintOption(self, mock_print):
        obj = chigger.base.ChiggerObject(name='andrew')
        obj.printOption('name')
        mock_print.assert_called_with("name='andrew'")

class TestChiggerObject(unittest.TestCase):
    def testSetOption(self):
        obj = chigger.base.ChiggerObject()
        t0 = obj._ChiggerObject__modified_time.GetMTime()

        obj.setParam('name', 'andrew')
        t1 = obj._ChiggerObject__modified_time.GetMTime()
        self.assertTrue(t1 > t0)

        obj.setParam('name', 'andrew')
        t2 = obj._ChiggerObject__modified_time.GetMTime()
        self.assertEqual(t1, t2)

    def testSetParams(self):
        obj = chigger.base.ChiggerObject()
        t0 = obj._ChiggerObject__modified_time.GetMTime()

        obj.setParams(name='andrew')
        t1 = obj._ChiggerObject__modified_time.GetMTime()
        self.assertTrue(t1 > t0)

        obj.setParams(name='andrew')
        t2 = obj._ChiggerObject__modified_time.GetMTime()
        self.assertEqual(t1, t2)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
