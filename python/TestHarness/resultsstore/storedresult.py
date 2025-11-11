# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement the StoredResult for storing a TestHarness result."""

from datetime import datetime
from typing import Optional

from bson.objectid import ObjectId

from TestHarness.resultsstore.utils import get_typed

NoneType = type(None)


class StoredResult:
    """
    Structure holding the information for a TestHarness result.

    This represents the data from in a .previous_test_results.json
    file from the TestHarness, stored within a mongo database.
    These objects are queried using the ResultsReader and stored
    using CIVETStore.

    It provides access to the information in the header using
    properties.
    """

    def __init__(
        self,
        data: dict,
        check: bool = True,
    ):
        """
        Initialize a result.

        Arguments:
        ---------
        data : dict
            The underyling data.

        Optional arguments:
        ------------------
        check : bool
            Whether or not to perform data type checking (default = True).

        """
        # The underlying data from the JSON results, which
        # is the dictionary representation of the entire
        # ".previous_test_results.json" file except for
        # the tests
        self._data: dict = data

        # Whether or not to validate data types on load
        self._check: bool = check

        # Sanity check on all of our methods (in order of definition)
        if self.check:
            self.check_data()

    def check_data(self):
        """Check the validity of the loaded data."""
        self.id
        self.testharness
        self.version
        self.validation_version
        self.civet
        self.civet_version
        self.civet_job_url
        self.civet_job_id
        self.hpc
        self.event_sha
        self.event_cause
        self.event_id
        self.pr_num
        self.base_sha
        self.time

    @property
    def check(self) -> bool:
        """Whether or not to validate data types on load."""
        return self._check

    @property
    def data(self) -> dict:
        """Get the underlying data."""
        return self._data

    @property
    def id(self) -> ObjectId:
        """Get the mongo database ID for these results."""
        return get_typed(self.data, "_id", ObjectId)

    @property
    def testharness(self) -> dict:
        """Get the testharness entry in the data."""
        return get_typed(self.data, "testharness", dict)

    @property
    def version(self) -> int:
        """Get the result version."""
        return get_typed(self.testharness, "version", int)

    @property
    def validation_version(self) -> int:
        """Get the validation version."""
        return get_typed(self.testharness, "validation_version", int, 0)

    @property
    def civet(self) -> dict:
        """
        Get the CIVET entry from the data.

        Contains information about the CIVET job that ran this test.
        """
        return get_typed(self.data, "civet", dict)

    @property
    def civet_version(self) -> int:
        """Get the CIVET schema version."""
        # Latest branch; civet_version in the data root
        if (
            civet_version := get_typed(
                self.data, "civet_version", int, allow_missing=True
            )
        ) is not None:
            return civet_version
        # Used to exist in ['civet']['version']
        if (version := self.civet.get("version")) is not None:
            return version
        # Before tracking, consider version 0
        return 0

    @property
    def civet_job_url(self) -> str:
        """Get the URL to the CIVET job that ran this test."""
        return get_typed(self.civet, "job_url", str)

    @property
    def civet_job_id(self) -> int:
        """Get the ID of the civet job that ran this test."""
        return get_typed(self.civet, "job_id", int)

    @property
    def hpc(self) -> dict:
        """Get the HPC entry that describes the HPC environment, if any."""
        return get_typed(self.data, "hpc", dict, {})

    @property
    def event_sha(self) -> str:
        """Get the commit that these tests were ran on."""
        return get_typed(self.data, "event_sha", str)

    @property
    def event_cause(self) -> str:
        """Get the cause for these tests that were ran."""
        return get_typed(self.data, "event_cause", str)

    @property
    def event_id(self) -> Optional[int]:
        """Get the ID of the civet event that ran this test."""
        return get_typed(self.data, "event_id", int) if self.civet_version > 2 else None

    @property
    def pr_num(self) -> Optional[int]:
        """Get the PR number associated with these tests (if any)."""
        return get_typed(self.data, "pr_num", (NoneType, int))

    @property
    def base_sha(self) -> Optional[str]:
        """Get the base commit that these tests were ran on."""
        return (
            get_typed(self.data, "base_sha", str) if self.civet_version >= 2 else None
        )

    @property
    def time(self) -> datetime:
        """Get the time these tests were added to the database."""
        return get_typed(self.data, "time", datetime)
