#!/usr/bin/env python2
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
from chigger.utils import Options

class TestOptions(unittest.TestCase):

    def assertInWarning(self, msg):
        output = sys.stdout.getvalue()
        found = False
        for match in re.finditer(r'WARNING\s*\n(?P<message>.*?)(?=^\n|\Z)', output, flags=re.MULTILINE|re.DOTALL):
            found = True
            self.assertIn(msg, match.group('message'))

        if not found:
            self.assertTrue(False, "No warnings exist.")

    def testMinimal(self):
        opts = Options()
        opts.add('foo')
        self.assertEqual(opts.keys(), ['foo'])
        self.assertFalse(opts.isOptionValid('foo'))
        self.assertIn('foo', opts)
        self.assertIsNone(opts.get('foo'))
        self.assertTrue(opts.hasOption('foo'))

    def testSet(self):
        opts = Options()
        opts.add('foo')
        opts.set('foo', 42)
        self.assertEqual(opts.keys(), ['foo'])
        self.assertTrue(opts.isOptionValid('foo'))
        self.assertIn('foo', opts)
        self.assertIsNotNone(opts.get('foo'))
        self.assertEqual(opts.get('foo'), 42)
        self.assertTrue(opts.hasOption('foo'))

    def testModified(self):
        opts = Options()
        opts.add('foo', 42)
        self.assertTrue(opts.modified())
        self.assertEqual(opts.applyOption('foo'), 42)
        self.assertFalse(opts.modified())
        opts.set('foo', 43)
        self.assertTrue(opts.modified())
        self.assertEqual(opts.applyOption('foo'), 43)
        self.assertFalse(opts.modified())

    def testSubOptions(self):
        sub_opts = Options()
        sub_opts.add('bar', 42)

        opts = Options()
        opts.add('foo', sub_opts)

        opts.set('foo', {'bar', 43})

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
