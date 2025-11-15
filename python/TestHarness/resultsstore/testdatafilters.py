# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Define filters that specify which data to load from tests."""

from enum import Enum
from typing import Iterable, Union


class TestDataFilter(Enum):
    """Filters for loading data entires in a test."""

    __test__ = False  # prevents pytest collection

    """Load all data in tests."""
    ALL = "all"

    """Load HPC data in tests."""
    HPC = "hpc"

    """Load the status entry in tests."""
    STATUS = "status"

    """Load the tester enter in tests."""
    TESTER = "tester"

    """Load the timing entry in tests."""
    TIMING = "timing"

    """Load the validation entry in tests."""
    VALIDATION = "validation"


def filter_as_iterable(
    test_filter: Union[Iterable[TestDataFilter], TestDataFilter],
) -> Iterable[TestDataFilter]:
    """
    If applicable, convert a single filter into an iterable filter.

    Useful for methods that can take a single filter or muliple filters.
    """
    return [test_filter] if isinstance(test_filter, TestDataFilter) else test_filter


def has_all_filter(test_filter: Iterable[TestDataFilter]) -> bool:
    """Whether or not the all filter is in the given filters."""
    return TestDataFilter.ALL in test_filter


# All of the potential keys stored within test data.
ALL_TEST_KEYS = [v.value for v in TestDataFilter if v != TestDataFilter.ALL]
