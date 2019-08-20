#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from MooseDocs import common
from MooseDocs.common import exceptions

class TestParseSettings(unittest.TestCase):
    def testBasic(self):
        defaults = dict(year=(1980, 'doc'), month=('june', 'doc'), day=(24, 'doc'))
        raw = 'year=2003'
        known, unknown = common.parse_settings(defaults, raw)
        self.assertEqual(known['year'], 2003)
        self.assertEqual(known['month'], 'june')
        self.assertEqual(known['day'], 24)
        self.assertEqual(unknown, dict())

    def testSpace(self):
        defaults = dict(year=(1980, 'doc'), month=('june', 'doc'), day=(24, 'doc'))
        raw = 'year=the year I was born'
        known, _ = common.parse_settings(defaults, raw)
        self.assertEqual(known['year'], 'the year I was born')

    def testFloat(self):
        defaults = dict(year=(1980, 'doc'))
        raw = 'year=2003'
        known, _ = common.parse_settings(defaults, raw)
        self.assertIsInstance(known['year'], float)

    def testUnknown(self):
        defaults = dict(year=(1980, 'doc'))
        raw = 'year=2003 month=june'
        known, unknown = common.parse_settings(defaults, raw, error_on_unknown=False)
        self.assertEqual(known['year'], 2003)
        self.assertNotIn('month', known)
        self.assertIn('month', unknown)
        self.assertEqual(unknown['month'], 'june')

    def testUnknownException(self):
        defaults = dict(year=(1980, 'doc'))
        raw = 'year=2003 month=june'
        with self.assertRaises(exceptions.MooseDocsException) as e:
            known, unknown = common.parse_settings(defaults, raw)

        self.assertIn("The following key, value settings are unknown:", str(e.exception))
        self.assertIn("month", str(e.exception))

    def testChangeException(self):
        defaults = dict(year=(1980, 'doc'))
        raw = 'year=2003 month=june'
        with self.assertRaises(exceptions.MooseDocsException) as e:
            known, unknown = common.parse_settings(defaults, raw)

        self.assertIn("The following key, value settings are unknown:", str(e.exception))
        self.assertIn("month", str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2)
