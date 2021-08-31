#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import os
import unittest
from mooseutils.MooseDeepDiff import MooseDeepDiff


class TestMooseDeepDiff(unittest.TestCase):
    def testInit(self):
        """Test Case: Check if the attributes of the MooseDeepDiff class are correctly set"""
        moose_diff = MooseDeepDiff([1.], [1.0000001], relative_error=1e-8)
        self.assertTrue(isinstance(moose_diff, MooseDeepDiff))
        self.assertEqual(moose_diff.rel_err, 1e-8)
        self.assertEqual(moose_diff.abs_err, None)
        self.assertEqual(moose_diff.t1, [1.])
        self.assertEqual(moose_diff.t2, [1.0000001])

    def testRelativeErrorDifference(self):
        """Test Case: A difference was identified by calculating the relative error"""
        moose_diff = MooseDeepDiff([1.], [1.0000001], relative_error=1e-8)
        has_changed = moose_diff.relative_error(1.0, 1.0000001, 1e-8)
        self.assertTrue(has_changed)

    def testRelativeErrorNoDifference(self):
        """Test Case: No difference was identified by calculating the relative error"""
        moose_diff = MooseDeepDiff([1.], [1.0000001], relative_error=1e-7)
        has_changed = moose_diff.relative_error(1.0, 1.0000001, 1e-7)
        self.assertFalse(has_changed)

    def testAbsoluteErrorDifference(self):
        """Test Case: A difference was identified by calculating the absolute error"""
        moose_diff = MooseDeepDiff([1.], [1.000000001], absolute_error=1e-10)
        has_changed = moose_diff.relative_error(1.0, 1.000000001, 1e-10)
        self.assertTrue(has_changed)

    def testAbsoluteErrorNoDifference(self):
        """Test Case: No difference was identified by calculating the absolute error"""
        moose_diff = MooseDeepDiff([1.], [1.000000001], absolute_error=1e-8)
        has_changed = moose_diff.relative_error(1.0, 1.000000001, 1e-8)
        self.assertFalse(has_changed)

    def testDiffNumbersRelativeError(self):
        """Test Case: A difference was identified by calculating the relative error. Report that the values changed"""
        moose_diff = MooseDeepDiff([1.], [1.0000001], relative_error=1e-8)
        expected_results = {
            'values_changed': {
                'root[0]': {
                    'new_value': 1.0000001,
                    'old_value': 1.0
                }
            }
        }
        self.assertEqual(moose_diff, expected_results)

    def testDiffNumbersAbsoluteError(self):
        """Test Case: A difference was identified by calculating the absolute error. Report that the values changed"""
        moose_diff = MooseDeepDiff([1.], [1.000000001], absolute_error=1e-10)
        expected_results = {
            'values_changed': {
                'root[0]': {
                    'new_value': 1.000000001,
                    'old_value': 1.0
                }
            }
        }
        self.assertEqual(moose_diff, expected_results)

    def testDiffNumbersEpsilon(self):
        """Test Case: A difference was identified by the math.isclose() method. Report that the values changed"""
        epsilon = 0.0001
        moose_diff = MooseDeepDiff([7.175], [7.174], math_epsilon=epsilon)
        expected_results = {'values_changed': {'root[0]': {'new_value': 7.174, 'old_value': 7.175}}}
        self.assertEqual(moose_diff, expected_results)

    def testDiffNumbersSignificantDigitsNone(self):
        """Test Case: A difference was identified when a value does not equal another value (significant_digits = None). Report that the values changed"""
        moose_diff = MooseDeepDiff([1.], [1.0000000001])
        expected_results = {
            'values_changed': {
                'root[0]': {
                    'new_value': 1.0000000001,
                    'old_value': 1.0
                }
            }
        }
        self.assertEqual(moose_diff, expected_results)

    def testDiffNumbersSignificantDigits(self):
        """Test Case: A difference was identified through string comparison via number_to_string(significant_digits, number_format_notation) method. Report that the values changed"""
        moose_diff = MooseDeepDiff([1.], [1.01], significant_digits=2, number_format_notation="f")
        expected_results = {'values_changed': {'root[0]': {'new_value': 1.01, 'old_value': 1.0}}}
        self.assertEqual(moose_diff, expected_results)


if __name__ == '__main__':
    unittest.main(module=__name__)
