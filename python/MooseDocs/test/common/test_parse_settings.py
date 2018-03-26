#!/usr/bin/env python2
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
        with self.assertRaises(exceptions.TokenizeException) as e:
            known, unknown = common.parse_settings(defaults, raw)

        self.assertIn("The following key, value settings are unknown:", e.exception.message)
        self.assertIn("month", e.exception.message)

    def testChangeException(self):
        defaults = dict(year=(1980, 'doc'))
        raw = 'year=2003 month=june'
        with self.assertRaises(KeyError) as e:
            known, unknown = common.parse_settings(defaults, raw, exc=KeyError)

        self.assertIn("The following key, value settings are unknown:", e.exception.message)
        self.assertIn("month", e.exception.message)



if __name__ == '__main__':
    unittest.main(verbosity=2)
