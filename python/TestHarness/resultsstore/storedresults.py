# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement the data classes in which results are stored for reading."""

import importlib
from copy import deepcopy
from datetime import datetime
from typing import Iterable, Optional

from bson.objectid import ObjectId

from TestHarness.resultsstore.testdatafilters import (
    TestDataFilter,
)
from TestHarness.resultsstore.utils import (
    TestName,
    decompress_dict,
    get_typed,
)
from TestHarness.validation.dataclasses import (
    ValidationData,
    ValidationDataTypesStr,
    ValidationResult,
)

# Load the data class module so that we can dynamically instantiate
# validation data types based on their type stored in json
validation_dataclasses_module = importlib.import_module(
    "TestHarness.validation.dataclasses"
)
for data_type in ValidationDataTypesStr:
    assert hasattr(validation_dataclasses_module, data_type)

NoneType = type(None)


class StoredTestResult:
    """
    Holds the data for a single test.

    It provides a convenient interface into one of the entries
    in the 'tests' key from a TestHarness execution, stored into
    a mongo database using the CIVETStore object.
    """

    def __init__(
        self,
        data: dict,
        name: TestName,
        result: "StoredResult",
        data_filters: Iterable[TestDataFilter],
    ):
        """
        Initialize the result.

        Arguments:
        ---------
        data : dict
            The underlying data for the test.
        name : TestName
            The name of the test.
        result : StoredResult
            The parent result.
        data_filters: Iterable[TestDataFilter]
            The filters that this test was built from.

        """
        assert isinstance(data, dict)
        assert isinstance(name, TestName)
        assert isinstance(result, StoredResult)

        # The underlying data for this test, which comes from
        # tests/<folder name>/tests/<test_name> in the TestHarness
        # JSON output
        self._data: dict = data
        # The combined name for this test (folder + name)
        self._name: TestName = name
        # The combined results that this test comes from
        self._result: "StoredResult" = result
        # Filters that this test was built from
        self._data_filters: Iterable[TestDataFilter] = data_filters

        if self.result.check:
            self.check_data()

    def check_data(self):
        """Check the validity of the loaded data."""
        self.id
        for filter in self._data_filters:
            match filter:
                case TestDataFilter.HPC:
                    self.hpc
                    self.hpc_id
                case TestDataFilter.STATUS:
                    self.status
                    self.status_value
                case TestDataFilter.TESTER:
                    self.tester
                    self.get_json_metadata()
                case TestDataFilter.TIMING:
                    self.timing
                    self.run_time
                    self.hpc_queued_time
                case TestDataFilter.VALIDATION:
                    self.validation
                    self.get_validation_data()
                    self.get_validation_results()

    def _require_filter(self, require: TestDataFilter):
        """Require that the given filter be used to access data."""
        if (
            require not in self._data_filters
            and TestDataFilter.ALL not in self._data_filters
        ):
            raise ValueError(f"Test data filter {require} required and not loaded")

    @property
    def data(self) -> dict:
        """Get the underlying data."""
        return self._data

    @property
    def id(self) -> Optional[ObjectId]:
        """
        Get the mongo database ID for this test.

        Could be None if the test is stored within the
        results data.
        """
        return get_typed(self.data, "_id", (NoneType, ObjectId))

    @property
    def result(self) -> "StoredResult":
        """Get the parent result that this test was ran in."""
        return self._result

    @property
    def result_id(self) -> ObjectId:
        """Get the mongo database ID the results."""
        return self.result.id

    @property
    def name(self) -> TestName:
        """Get the combined name for this test (folder + name)."""
        return self._name

    @property
    def folder_name(self) -> str:
        """Get the name of the folder the test is in."""
        return self.name.folder

    @property
    def test_name(self) -> str:
        """Get the name of the test."""
        return self.name.name

    @property
    def status(self) -> Optional[dict]:
        """
        Get the status entry for the test.

        Requires filter TestDataFilter.STATUS when loading tests.
        """
        self._require_filter(TestDataFilter.STATUS)
        return get_typed(self.data, "status", (NoneType, dict))

    @property
    def status_value(self) -> Optional[str]:
        """
        Get the status value for the test (OK, ERROR, etc).

        Requires filter TestDataFilter.STATUS when loading tests.
        """
        return get_typed(status, "status", str) if (status := self.status) else None

    @property
    def timing(self) -> Optional[dict]:
        """
        Get the timing entry for the test.

        Requires filter TestDataFilter.TIMING when loading tests.
        """
        self._require_filter(TestDataFilter.TIMING)
        return get_typed(self.data, "timing", (dict, NoneType))

    def get_timing_entry(self, name: str) -> Optional[float]:
        """
        Get the named timing entry if it exists.

        Requires filter TestDataFilter.TIMING when loading tests.
        """
        return (
            get_typed(timing, name, (float, NoneType))
            if (timing := self.timing)
            else None
        )

    @property
    def run_time(self) -> Optional[float]:
        """
        Get the run time for this test (if available).

        Requires filter TestDataFilter.TIMING when loading tests.
        """
        return self.get_timing_entry("runner_run")

    @property
    def hpc_queued_time(self) -> Optional[float]:
        """
        Get the time that this test was queued on HPC.

        If the test was not ran on HPC, this will be None.

        Requires filter TestDataFilter.TIMING when loading tests.
        """
        return self.get_timing_entry("hpc_queued_time")

    @property
    def event_sha(self) -> str:
        """Get the commit that this test was ran on."""
        return self.result.event_sha

    @property
    def event_cause(self) -> str:
        """Get the cause for the test that was ran."""
        return self.result.event_cause

    @property
    def event_id(self) -> Optional[int]:
        """Get the ID of the event that this job was ran on."""
        return self.result.event_id

    @property
    def pr_num(self) -> Optional[int]:
        """Get the PR number associated with the test (if any)."""
        return self.result.pr_num

    @property
    def base_sha(self) -> Optional[str]:
        """Get the base commit that these tests were ran on."""
        return self.result.base_sha

    @property
    def time(self) -> datetime:
        """Get the time this test was added to the database."""
        return self.result.time

    @property
    def tester(self) -> Optional[dict]:
        """
        Get the Tester entry in the data.

        Requires filter TestDataFilter.TESTER when loading tests.
        """
        self._require_filter(TestDataFilter.TESTER)
        return get_typed(self.data, "tester", (NoneType, dict))

    @property
    def hpc(self) -> Optional[dict]:
        """
        Get the 'hpc' entry if it exists.

        Requires filter TestDataFilter.HPC when loading tests.
        """
        self._require_filter(TestDataFilter.HPC)
        return get_typed(self.data, "hpc", (NoneType, dict))

    @property
    def hpc_id(self) -> Optional[int]:
        """
        Get the HPC job ID that ran this this test, if any.

        Requires filter TestDataFilter.HPC when loading tests.
        """
        return get_typed(hpc, "id", int) if (hpc := self.hpc) else None

    @property
    def validation(self) -> Optional[dict]:
        """
        Get the 'validation' entry for the test, if any.

        Contains the validation data and results. The methods
        get_validation_data() and get_validation_results()
        should be preferred over this for getting the pythonic
        representation of the results and data.

        Requires filter TestDataFilter.VALIDATION when loading tests.
        """
        self._require_filter(TestDataFilter.VALIDATION)
        return get_typed(self.data, "validation", (NoneType, dict))

    def get_validation_data(self) -> dict[str, ValidationData]:
        """
        Get the built ValidationData objects for this test, if any.

        Requires filter TestDataFilter.VALIDATION when loading tests.
        """
        validation_data = {}
        if validation := self.validation:
            input = deepcopy(validation.get("data", {}))

            for k, v in input.items():
                # Bounds is saved as a list; convert to tuple
                if v.get("bounds") is not None:
                    v["bounds"] = tuple(v["bounds"])

                # Before version 1, the data type was not stored
                if self.result.validation_version == 0:
                    data_type = "ValidationScalarData"
                # After version 1, the type is explicitly available
                else:
                    data_type = v.pop("type")
                if data_type not in ValidationDataTypesStr:
                    raise ValueError(
                        f"Unknown validation data type {data_type} for data '{k}'"
                    )

                # Build the underlying data class instead of a dict
                validation_data[k] = getattr(validation_dataclasses_module, data_type)(
                    **v
                )

        return validation_data

    def get_validation_results(self) -> list[ValidationResult]:
        """
        Get the build ValidationResult objects for this test, if any.

        Requires filter TestDataFilter.VALIDATION when loading tests.
        """
        return (
            [ValidationResult(**v) for v in deepcopy(validation.get("results", []))]
            if (validation := self.validation)
            else []
        )

    def get_json_metadata(self) -> dict[str, dict]:
        """
        Get the JSON metadata from the Tester entry, if any.

        Requires filter TestDataFilter.TESTER when loading tests.
        """
        json_metadata = {}

        if (tester := self.tester) and (
            json_metadata := get_typed(tester, "json_metadata", dict)
        ):
            for k, v in json_metadata.items():
                assert isinstance(v, bytes)
                try:
                    loaded = decompress_dict(v)
                except Exception:
                    raise ValueError(
                        f"Failed to decompress json_metadata {k} "
                        f"in test {self.name}"
                    )
                json_metadata[k] = loaded

        return json_metadata

    def get_perf_graph(self) -> Optional[dict]:
        """
        Get the perf_graph entry in the JSON metadata, if any.

        Requires filter TestDataFilter.TESTER when loading tests.
        """
        return get_typed(self.get_json_metadata(), "perf_graph", (NoneType, dict))


class StoredResult:
    """
    Structure holding the information for a TestHarness result.

    This represents the data from in a .previous_test_results.json
    file from the TestHarness, stored within a mongo database.
    These objects are queried using the ResultsReader and stored
    using CIVETStore.

    It provides access to the information in the header using
    various properties and access to the contained tests
    via get_tests() and the tests property.

    Each individual test can either be stored within this
    result's data, or within a separate mongo collection.
    If the test data is stored separately, this object
    will automatically query the data as needed.
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
        if (civet_version := get_typed(self.data, "civet_version", int)) is not None:
            return civet_version
        # Used to exist in ['civet']['version']
        if (civet := self.data.get("civet")) and (
            version := civet.get("version")
        ) is not None:
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
