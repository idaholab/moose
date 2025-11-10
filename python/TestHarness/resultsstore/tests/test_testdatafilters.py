# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.testdatafilters."""

from unittest import TestCase

from TestHarness.resultsstore.testdatafilters import ALL_TEST_KEYS, TestDataFilter


class TestCIVETStore(TestCase):
    """Test TestHarness.resultsstore.testdatafilter."""

    def test_TestDataFilter(self):
        """Test TestDataFilter."""
        self.assertEqual(TestDataFilter.ALL.value, "all")
        self.assertEqual(TestDataFilter.HPC.value, "hpc")
        self.assertEqual(TestDataFilter.STATUS.value, "status")
        self.assertEqual(TestDataFilter.TESTER.value, "tester")
        self.assertEqual(TestDataFilter.TIMING.value, "timing")
        self.assertEqual(TestDataFilter.VALIDATION.value, "validation")

    def test_ALL_TEST_KEYS(self):
        """Test ALL_TEST_KEYS."""
        self.assertEqual(len(ALL_TEST_KEYS), len(TestDataFilter) - 1)
