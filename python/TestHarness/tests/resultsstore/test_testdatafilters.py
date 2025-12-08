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

from TestHarness.resultsstore.testdatafilters import (
    ALL_TEST_KEYS,
    TestDataFilter,
    filter_as_iterable,
    has_all_filter,
)


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

    def test_filter_as_iterable(self):
        """Test filter_as_iterable()."""
        test_filter = TestDataFilter.STATUS

        # Does a conversion
        to_list = filter_as_iterable(test_filter)
        assert isinstance(to_list, list)
        self.assertEqual(len(to_list), 1)
        self.assertEqual(to_list[0], test_filter)

        # Doesn't do a conversion
        as_set = set([test_filter])
        same = filter_as_iterable(as_set)
        self.assertIs(as_set, same)

    def test_has_all_filter(self):
        """Test has_all_filter()."""
        self.assertFalse(has_all_filter([TestDataFilter.HPC, TestDataFilter.STATUS]))
        self.assertTrue(has_all_filter([TestDataFilter.STATUS, TestDataFilter.ALL]))

    def test_ALL_TEST_KEYS(self):
        """Test ALL_TEST_KEYS."""
        self.assertEqual(len(ALL_TEST_KEYS), len(TestDataFilter) - 1)
