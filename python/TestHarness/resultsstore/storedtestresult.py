# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement the StoredTestResult for storing a single test result."""

import importlib
from copy import deepcopy
from datetime import datetime
from typing import Iterable, Optional, Tuple

from bson.objectid import ObjectId
from moosepy.perfgraphreader import PerfGraphReader

from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.testdatafilters import (
    TestDataFilter,
)
from TestHarness.resultsstore.utils import (
    TestName,
    compress_dict,
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
        result: StoredResult,
        data_filters: Iterable[TestDataFilter],
    ):
        """
        Initialize the result.

        Parameters
        ----------
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

    check_values: dict[TestDataFilter, Tuple[str, ...]] = {
        TestDataFilter.HPC: ("hpc", "hpc_id")
    }

    def check_data(self):
        """Check the validity of the loaded data."""
        self.id

        has_all = TestDataFilter.ALL in self._data_filters

        for filter in self._data_filters:
            if filter == TestDataFilter.HPC or has_all:
                self.hpc
                self.hpc_id
            if filter == TestDataFilter.STATUS or has_all:
                self.status
                self.status_value
            if filter == TestDataFilter.TESTER or has_all:
                self.tester
                self.get_json_metadata()
            if filter == TestDataFilter.TIMING or has_all:
                self.timing
                self.run_time
                self.hpc_queued_time
            if filter == TestDataFilter.VALIDATION or has_all:
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
    def hpc_id(self) -> Optional[str]:
        """
        Get the HPC job ID that ran this this test, if any.

        Requires filter TestDataFilter.HPC when loading tests.
        """
        return get_typed(hpc, "id", str) if (hpc := self.hpc) else None

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
                if (bounds := v.get("bounds")) is not None:
                    v["bounds"] = tuple(bounds)

                # Before version 1, the data type was not stored
                if self.result.validation_version == 0:
                    data_type = "ValidationScalarData"
                # After version 1, the type is explicitly available
                else:
                    data_type = v.pop("type")
                if data_type not in ValidationDataTypesStr:
                    raise ValueError(
                        f"Unknown validation data type '{data_type}' for data '{k}'"
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
        values = {}

        if (tester := self.tester) and (
            json_metadata := get_typed(tester, "json_metadata", (NoneType, dict))
        ):
            for k, v in json_metadata.items():
                assert isinstance(v, bytes)
                try:
                    loaded = decompress_dict(v)
                except Exception as e:
                    raise ValueError(
                        f"Failed to decompress json_metadata {k} in test {self.name}"
                    ) from e
                values[k] = loaded

        return values

    def get_perf_graph(self) -> Optional[PerfGraphReader]:
        """
        Get the perf_graph entry in the JSON metadata, if any.

        Requires filter TestDataFilter.TESTER when loading tests.
        """
        reporter_data = get_typed(
            self.get_json_metadata(), "perf_graph", (NoneType, dict)
        )
        if reporter_data is None:
            return None
        data = reporter_data["time_steps"][-1]["perf_graph_json"]["graph"]
        if data is None:
            return None
        return PerfGraphReader(data)

    @property
    def max_memory(self) -> Optional[int]:
        """Get the estimated max memory usage for the test in bytes, if available."""
        return get_typed(self.data, "max_memory", (NoneType, int))

    def serialize(self) -> dict:
        """
        Serialize the data so that it is JSON dumpable.

        Can be reloaded with deserialize().
        """
        data = deepcopy(self.data)

        data["serialized"] = {
            "folder_name": self.folder_name,
            "test_name": self.test_name,
            "data_filters": [v.name for v in self._data_filters],
        }

        # Convert ObjectID to string ID
        for key in ["_id", "result_id"]:
            if value := data.get(key):
                data[key] = str(value)

        # Convert binary JSON metadata to dict
        if json_metadata := self.get_json_metadata():
            data["tester"]["json_metadata"] = json_metadata

        # Time entry if it exists
        if (time := data.get("time")) is not None:
            data["time"] = str(time)

        return data

    @staticmethod
    def deserialize(data: dict, result: StoredResult) -> "StoredTestResult":
        """Deserialize data from serialize() into a test result."""
        assert isinstance(data, dict)
        assert isinstance(result, StoredResult)

        serialized = data["serialized"]
        name = TestName(serialized["folder_name"], serialized["test_name"])
        filters = [TestDataFilter[v] for v in serialized["data_filters"]]
        del data["serialized"]

        # Convert string ID to ObjectID
        for key in ["_id", "result_id"]:
            if value := data.get(key):
                data[key] = ObjectId(value)

        # Convert JSON metadata back to binary
        tester = data.get("tester", {})
        json_metadata = tester.get("json_metadata")
        if json_metadata:
            data["tester"]["json_metadata"] = {
                k: compress_dict(v) for k, v in json_metadata.items()
            }

        # Convert time to datettime
        if (time := data.get("time")) is not None:
            data["time"] = datetime.fromisoformat(time)

        return StoredTestResult(data, name, result, filters)
