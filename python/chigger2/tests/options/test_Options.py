#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import re
import unittest
from chigger.utils import Params

class TestParams(unittest.TestCase):

    def assertInWarning(self, msg):
        output = sys.stdout.getvalue()
        found = False
        for match in re.finditer(r'WARNING\s*\n(?P<message>.*?)(?=^\n|\Z)', output, flags=re.MULTILINE|re.DOTALL):
            found = True
            self.assertIn(msg, match.group('message'))

        if not found:
            self.assertTrue(False, "No warnings exist.")

    def testMinimal(self):
        opts = Params()
        opts.add('foo')
        self.assertEqual(opts.keys(), ['foo'])
        self.assertFalse(opts.isValid('foo'))
        self.assertIn('foo', opts)
        self.assertIsNone(opts.getValue('foo'))
        self.assertTrue(opts.hasParameter('foo'))

    def testSet(self):
        opts = Params()
        opts.add('foo')
        opts.setValue('foo', 42)
        self.assertEqual(opts.keys(), ['foo'])
        self.assertTrue(opts.isValid('foo'))
        self.assertIn('foo', opts)
        self.assertIsNotNone(opts.getValue('foo'))
        self.assertEqual(opts.getValue('foo'), 42)
        self.assertTrue(opts.hasParameter('foo'))

    def testModified(self):
        opts = Params()
        opts.add('foo', 42)
        self.assertTrue(opts.modified())
        self.assertEqual(opts.applyOption('foo'), 42)
        self.assertFalse(opts.modified())
        opts.setValue('foo', 43)
        self.assertTrue(opts.modified())
        self.assertEqual(opts.applyOption('foo'), 43)
        self.assertFalse(opts.modified())

    def testSubParams(self):
        sub_opts = Params()
        sub_opts.add('bar', 42)

        opts = Params()
        opts.add('foo', sub_opts)

        opts.setValue('foo', {'bar', 43})

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
