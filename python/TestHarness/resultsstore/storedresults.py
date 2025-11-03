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
from typing import Optional, Union

from bson.objectid import ObjectId
from pymongo.database import Database

from TestHarness.resultsstore.utils import (
    TestName,
    compress_dict,
    decompress_dict,
    results_folder_iterator,
    results_test_iterator,
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


class DatabaseException(Exception):
    """Exception for an error querying data in the database."""

    pass


class StoredTestResult:
    """
    Holds the data for a single test.

    It provides a convenient interface into one of the entries
    in the 'tests' key from a TestHarness execution, stored into
    a mongo database using the CIVETStore object.
    """

    def __init__(self, data: dict, name: TestName, result: "StoredResult"):
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

        """
        # The underlying data for this test, which comes from
        # tests/<folder name>/tests/<test_name> in the TestHarness
        # JSON output
        self._data: dict = data
        # The combined name for this test (folder + name)
        self._name: TestName = name
        # The combined results that this test comes from
        self._results: "StoredResult" = result

        # Sanity check on all of our data and methods
        assert isinstance(self.data, dict)
        assert isinstance(self.id, (ObjectId, NoneType))
        assert isinstance(self.results, StoredResult)
        assert isinstance(self.result_id, ObjectId)
        assert isinstance(self.name, TestName)
        assert isinstance(self.test_name, str)
        assert isinstance(self.folder_name, str)
        assert isinstance(self.status, (dict, NoneType))
        assert isinstance(self.status_value, (str, NoneType))
        assert isinstance(self.timing, (dict, NoneType))
        assert isinstance(self.run_time, (float, NoneType))
        assert isinstance(self.hpc_queued_time, (float, NoneType))
        assert isinstance(self.event_sha, str)
        assert len(self.event_sha) == 40
        assert isinstance(self.event_cause, str)
        assert isinstance(self.pr_num, (int, NoneType))
        assert isinstance(self.base_sha, (str, NoneType))
        if self.base_sha:
            assert len(self.base_sha) == 40
        assert isinstance(self.time, datetime)
        assert isinstance(self.tester, (dict, NoneType))
        assert isinstance(self.hpc, (dict, NoneType))
        assert isinstance(self.hpc_id, (str, NoneType))
        assert isinstance(self.validation, (dict, NoneType))

        # Load the validation results and data
        self._validation_results: Optional[list[ValidationResult]] = (
            self._buildValidationResults()
        )
        self._validation_data: Optional[dict[str, ValidationData]] = (
            self._buildValidationData()
        )
        assert isinstance(self.validation_results, (list, NoneType))
        assert isinstance(self.validation_data, (dict, NoneType))

        # Load JSON metadata; this needs to be done separately
        # because metadata is stored in a compressed (with zlib)
        # form as it can be large
        self._json_metadata: Optional[dict[str, dict]] = self._buildJSONMetadata()
        assert isinstance(self.json_metadata, (dict, NoneType))
        assert isinstance(self.perf_graph, (dict, NoneType))

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
        return self.data.get("_id")

    @property
    def results(self) -> "StoredResult":
        """Get the combined results that this test comes from."""
        return self._results

    @property
    def result_id(self) -> ObjectId:
        """Get the mongo database ID the results."""
        # Will be none if this test data exists in results
        result_id = self.data.get("result_id")
        if result_id is not None:
            assert self.results.id == result_id
        return self.results.id

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
        """Get the status entry for the test."""
        return self.data.get("status")

    @property
    def status_value(self) -> Optional[str]:
        """Get the status value for the test (OK, ERROR, etc)."""
        return self.status["status"] if self.status is not None else None

    @property
    def timing(self) -> Optional[dict]:
        """Get the timing entry for the test."""
        return self.data.get("timing")

    @property
    def run_time(self) -> Optional[float]:
        """Get the run time for this test (if available)."""
        return self.timing.get("runner_run") if self.timing is not None else None

    @property
    def hpc_queued_time(self) -> Optional[float]:
        """
        Get the time that this test was queued on HPC.

        If the test was not ran on HPC, this will be None.
        """
        return self.timing.get("hpc_queued") if self.timing is not None else None

    @property
    def event_sha(self) -> str:
        """Get the commit that this test was ran on."""
        return self.results.event_sha

    @property
    def event_cause(self) -> str:
        """Get the cause for the test that was ran."""
        return self.results.event_cause

    @property
    def event_id(self) -> Optional[int]:
        """Get the ID of the event that this job was ran on."""
        return self.results.event_id

    @property
    def pr_num(self) -> Optional[int]:
        """Get the PR number associated with the test (if any)."""
        return self.results.pr_num

    @property
    def base_sha(self) -> Optional[str]:
        """Get the base commit that these tests were ran on."""
        return self.results.base_sha

    @property
    def time(self) -> datetime:
        """Get the time this test was added to the database."""
        return self.results.time

    @property
    def tester(self) -> Optional[dict]:
        """Get the Tester entry in the data."""
        return self.data.get("tester")

    @property
    def hpc(self) -> Optional[dict]:
        """Get the 'hpc' entry if it exists."""
        return self.data.get("hpc")

    @property
    def hpc_id(self) -> Optional[int]:
        """Get the HPC job ID that ran this this test, if any."""
        return self.hpc.get("id") if self.hpc is not None else None

    @property
    def validation(self) -> Optional[dict]:
        """
        Get the 'validation' entry for the test, if any.

        Contains the validation data and results.
        """
        return self.data.get("validation")

    @property
    def validation_data(self) -> Optional[dict[str, ValidationData]]:
        """Get the 'data' entry in 'validation' for this test, if any."""
        return self._validation_data

    @property
    def validation_results(self) -> Optional[list[ValidationResult]]:
        """Get the 'data' entry in 'validation' for this test, if any."""
        return self._validation_results

    def _buildValidationResults(self) -> Optional[list[ValidationResult]]:
        """
        Convert validation results in JSON to the underlying objects.

        Used on construction.
        """
        if self.validation is not None:
            return [
                ValidationResult(**v)
                for v in deepcopy(self.validation.get("results", []))
            ]
        return None

    def _buildValidationData(self) -> Optional[dict[str, ValidationData]]:
        """
        Convert valiation data in JSON to the underlying objects.

        Used on construction.
        """
        if self.validation is None:
            return None

        input = deepcopy(self.validation.get("data", {}))
        data = {}

        for k, v in input.items():
            # Bounds is saved as a list; convert to tuple
            if v.get("bounds") is not None:
                v["bounds"] = tuple(v["bounds"])

            # Before version 1, the data type was not stored
            if self.results.validation_version == 0:
                data_type = "ValidationScalarData"
            # After version 1, the type is explicitly available
            else:
                data_type = v.pop("type")
            if data_type not in ValidationDataTypesStr:
                raise ValueError(
                    f"Unknown validation data type {data_type} for data '{k}'"
                )

            # Build the underlying data class instead of a dict
            data[k] = getattr(validation_dataclasses_module, data_type)(**v)

        return data

    @property
    def json_metadata(self) -> Optional[dict[str, dict]]:
        """Get the 'json_metadata' entry from the Tester entry, if any."""
        return self._json_metadata

    def _buildJSONMetadata(self) -> Optional[dict[str, dict]]:
        """
        Build the tester.json_metadata entry.

        This is needed because JSON metadata is stored as
        a compressed binary string in data, in which here
        we will decompress the underlying data into its
        original dict.

        Used on construction.
        """
        # No metadata without a tester (stored in the tester)
        if self.tester is None:
            return None
        # If no json_metadata key, there was none
        json_metadata = self.tester.get("json_metadata")
        if json_metadata is None:
            return None

        assert isinstance(json_metadata, dict)

        json_metadata_decompressed = {}
        for k, v in json_metadata.items():
            assert isinstance(v, bytes)
            try:
                loaded = decompress_dict(v)
            except Exception:
                raise ValueError(
                    f"Failed to decompress json_metadata {k} " f"in test {self.name}"
                )
            json_metadata_decompressed[k] = loaded
        return json_metadata_decompressed

    @property
    def perf_graph(self) -> Optional[dict]:
        """Get the perf_graph entry in the JSON metadata, if any."""
        return self.json_metadata.get("perf_graph") if self.json_metadata else None


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

    def __init__(self, data: dict, db: Optional[Database] = None):
        """
        Initialize a result.

        Arguments:
        ---------
        data : dict
            The underyling data.
        db : Optional[Database]
            The database to query additional data from.

        """
        # The underlying data from the JSON results, which
        # is the dictionary representation of the entire
        # ".previous_test_results.json" file
        assert isinstance(data, dict)
        self._data: dict = data

        # The database connection, used for querying test results
        assert isinstance(db, (Database, NoneType))
        self._db: Optional[Database] = db

        # Initialize empty test objects
        self._tests = self.init_tests(data)

        # Sanity check on all of our methods (in order of definition)
        assert isinstance(self.data, dict)
        assert isinstance(self.id, ObjectId)
        assert isinstance(self.testharness, dict)
        assert isinstance(self.version, int)
        assert isinstance(self.validation_version, int)
        assert isinstance(self.civet, dict)
        assert isinstance(self.civet_version, int)
        assert isinstance(self.civet_job_url, str)
        assert isinstance(self.civet_job_id, int)
        assert isinstance(self.hpc, dict)
        assert isinstance(self.event_sha, str)
        assert len(self.event_sha) == 40
        assert isinstance(self.event_cause, str)
        assert isinstance(self.event_id, (int, NoneType))
        assert isinstance(self.pr_num, (int, NoneType))
        assert isinstance(self.base_sha, (str, NoneType))
        if self.base_sha:
            assert len(self.base_sha) == 40
        assert isinstance(self.time, datetime)
        assert isinstance(self.num_tests, int)
        assert isinstance(self.test_names, list)
        for v in self.test_names:
            assert isinstance(v, TestName)

    def init_tests(
        self, results: dict
    ) -> dict[TestName, Union[StoredTestResult, ObjectId]]:
        """
        Build the tests to be stored within the _tests member.

        This is a dict from test (folder name, test name) to either:
            - An ObjectID, if the test is stored in a separate collection
            - A StoredTestResult, if the test is stored directly in the results

        When an object is stored as an ObjectID (the ID to the test
        in the "tests" mongo collection), it can be queried later
        on demand as needed.
        """
        assert isinstance(results, dict)

        values: dict[TestName, Union[StoredTestResult, ObjectId]] = {}

        all_ids = set()
        for test in results_test_iterator(results):
            name = test.name
            assert name not in values
            test_entry = test.value

            # References a test in a seprate collection
            # Store the id to be pulled later if needed
            if isinstance(test_entry, ObjectId):
                assert test_entry not in all_ids
                value = test_entry
                all_ids.add(test_entry)
            # Is direct test data
            # Store the actual test object
            elif isinstance(test_entry, dict):
                value = self._build_test(test_entry, name)
            # Unknown type
            else:
                raise TypeError(
                    f'Test "{name}" entry in result._id={self.id} '
                    f'has unexpected type "{type(test_entry).__name__}"'
                )

            values[name] = value

        return values

    @property
    def data(self) -> dict:
        """Get the underlying data."""
        return self._data

    @property
    def id(self) -> ObjectId:
        """Get the mongo database ID for these results."""
        return self.data["_id"]

    @property
    def testharness(self) -> dict:
        """Get the testharness entry in the data."""
        return self.data["testharness"]

    @property
    def version(self) -> int:
        """Get the result version."""
        return self.testharness["version"]

    @property
    def validation_version(self) -> int:
        """Get the validation version."""
        return self.testharness.get("validation_version", 0)

    @property
    def civet(self) -> dict:
        """
        Get the CIVET entry from the data.

        Contains information about the CIVET job that ran this test.
        """
        return self.data["civet"]

    @property
    def civet_version(self) -> int:
        """Get the CIVET schema version."""
        # Version was first added to ['civet']['version'], and
        # was then moved to ['civet_version']
        version = self.civet.get("version", self.data.get("civet_version", 0))
        assert isinstance(version, int)
        return version

    @property
    def civet_job_url(self) -> str:
        """Get the URL to the CIVET job that ran this test."""
        return self.civet["job_url"]

    @property
    def civet_job_id(self) -> int:
        """Get the ID of the civet job that ran this test."""
        return self.civet["job_id"]

    @property
    def hpc(self) -> dict:
        """Get the HPC entry that describes the HPC environment, if any."""
        return self.data.get("hpc", {})

    @property
    def event_sha(self) -> str:
        """Get the commit that these tests were ran on."""
        return self.data["event_sha"]

    @property
    def event_cause(self) -> str:
        """Get the cause for these tests that were ran."""
        return self.data["event_cause"]

    @property
    def event_id(self) -> Optional[int]:
        """Get the ID of the civet event that ran this test."""
        if self.civet_version > 2:
            id = self.data["event_id"]
            assert isinstance(id, int)
            return id
        return None

    @property
    def pr_num(self) -> Optional[int]:
        """Get the PR number associated with these tests (if any)."""
        return self.data["pr_num"]

    @property
    def base_sha(self) -> Optional[str]:
        """Get the base commit that these tests were ran on."""
        if self.civet_version < 2:
            return None
        return self.data["base_sha"]

    @property
    def time(self) -> datetime:
        """Get the time these tests were added to the database."""
        return self.data["time"]

    @property
    def num_tests(self) -> int:
        """Get the number of stored tests."""
        return len(self._tests)

    @property
    def test_names(self) -> list[TestName]:
        """Get the combined names of all tests."""
        return list(self._tests.keys())

    def has_test(self, folder_name: str, test_name: str) -> bool:
        """
        Whether or not a test with the given folder and test name is stored.

        Parameters
        ----------
        folder_name : str
            The name of the folder for the test
        test_name : str
            The name of the test

        """
        assert isinstance(folder_name, str)
        assert isinstance(test_name, str)

        key = TestName(folder_name, test_name)
        return key in self._tests

    def _find_test_data(self, id: ObjectId) -> Optional[dict]:
        """
        Get the data assocated with a test.

        This is a separate function so that it can be mocked
        easily within unit tests.

        It queries the test database for a test that matches
        the given ID.
        """
        assert isinstance(self._db, Database)
        assert isinstance(id, ObjectId)

        return self._db.tests.find_one({"_id": id})

    def _build_test(self, data: dict, name: TestName) -> StoredTestResult:
        """
        Build a StoredTestResult.

        Exists so that failures to build a StoredTestResult
        (common when types conflict) are wrapped with
        an exception with more context.
        """
        assert isinstance(data, dict)
        assert isinstance(name, TestName)

        # Build the true object from the data
        try:
            return StoredTestResult(data, name, self)
        except Exception as e:
            id = data.get("_id")
            raise ValueError(f"Failed to build test result id={id}") from e

    def get_test(self, folder_name: str, test_name: str) -> StoredTestResult:
        """
        Get the test result associated with the given folder and name.

        Parameters
        ----------
        folder_name : str
            The name of the folder for the test
        test_name : str
            The name of the test

        """
        assert isinstance(folder_name, str)
        assert isinstance(test_name, str)

        # Search for the data in the cache
        name = TestName(folder_name, test_name)
        value = self._tests.get(name)

        # Doesn't exist
        if value is None:
            raise KeyError(f'Test "{name}" does not exist')

        # Is already built
        if isinstance(value, StoredTestResult):
            return value

        # Not built, so pull from database
        assert isinstance(value, ObjectId)
        data = self._find_test_data(value)
        if data is None:
            raise DatabaseException(f"Database missing tests._id={value}")

        # Build the true object from the data
        test_result = self._build_test(data, name)

        # And store in the cache
        self._tests[name] = test_result

        return test_result

    def _find_tests_data(self, ids: list[ObjectId]) -> list[dict]:
        """
        Get the data associated with tests in the database.

        This is a separate function so that it can be mocked
        easily within unit tests.

        It queries the tests database for all documents that match
        the given IDs.
        """
        assert self._db is not None
        assert isinstance(self._db, Database)
        assert isinstance(ids, list)

        with self._db.tests.find({"_id": {"$in": ids}}) as cursor:
            return [doc for doc in cursor]

    def load_all_tests(self):
        """Load all tests that have not been loaded from the database."""
        # Get tests that need to be loaded; need a mapping
        # of id -> name so that we can go from a document
        # to a name when obtaining data from the database
        tests = {
            id: name for name, id in self._tests.items() if isinstance(id, ObjectId)
        }

        # Nothing to load
        if not tests:
            return

        # Load and build everything we need
        for doc in self._find_tests_data(list(tests.keys())):
            id = doc["_id"]
            name = tests[id]
            self._tests[name] = self._build_test(doc, name)

        # Make sure we found every test
        missing = [
            str(entry) for entry in self._tests.values() if isinstance(entry, ObjectId)
        ]
        if missing:
            raise KeyError(f"Failed to load test results for _id={missing}")

    def get_tests(self) -> list[StoredTestResult]:
        """
        Get all of the test results.

        Will load all tests that have not been loaded from the database.
        """
        self.load_all_tests()
        return list(self._tests.values())

    def serialize(self, test_filter: Optional[list[TestName]] = None) -> dict:
        """
        Serialize this result.

        Enables be storage and later reconstruction
        using deserialize/deserialize_build.

        Used primarily in unit testing.

        Optional Parameters
        -------------------
        test_filter : Optional[list[TestName]]
            Only store these named tests, if provided.
        """
        # Load all tests so that they can be stored
        self.load_all_tests()

        # Copy our data
        data = deepcopy(self.data)

        # Convert datetime to str that can be converted back
        data["time"] = str(data["time"])
        # Convert ObjectID to string id
        data["_id"] = str(data["_id"])

        for folder in results_folder_iterator(data):
            for test in folder.test_iterator():
                # Filter and not in the filter
                if test_filter and test.name not in test_filter:
                    test.delete()
                    continue

                # Duplicate test data for storage
                test_data = deepcopy(
                    self.get_test(test.folder_name, test.test_name).data
                )

                # Convert ObjectID to string ID
                for key in ["_id", "result_id"]:
                    if key in test_data:
                        test_data[key] = str(test_data[key])

                # Convert binary JSON metadata to dict
                json_metadata = test_data.get("tester", {}).get("json_metadata", {})
                for k, v in json_metadata.items():
                    json_metadata[k] = decompress_dict(v)

                # Time entry existed before civet version 4
                if self.civet_version < 4:
                    test_data["time"] = str(test_data["time"])

                test.set_value(test_data)

            folder.delete_if_empty()

        return data

    @staticmethod
    def deserialize(data: dict) -> dict:
        """
        Deserialize data built with serialize().

        Can be used to construct a StoredResult object.

        Used primarily in unit testing.
        """
        assert isinstance(data, dict)

        # Convert datetime str to datetime object
        data["time"] = datetime.fromisoformat(data["time"])
        # Convert string id to ObjectID
        data["_id"] = ObjectId(data["_id"])

        # Convert each test
        for test in results_test_iterator(data):
            test_entry = test.value

            # Convert string id to ObjectID
            for key in ["_id", "result_id"]:
                if key in test_entry:
                    test_entry[key] = ObjectId(test_entry[key])

            # Convert string JSON metadata to binary
            json_metadata = test_entry.get("tester", {}).get("json_metadata", {})
            for k, v in json_metadata.items():
                json_metadata[k] = compress_dict(v)

            # Convert time if it exists (removed after
            # civet_version 3)
            if "time" in test_entry:
                test_entry["time"] = datetime.fromisoformat(test_entry["time"])

        return data

    @staticmethod
    def deserialize_build(data: dict) -> "StoredResult":
        """
        Build a StoredResult using data serialized with serialize().

        Used primarily in unit testing.
        """
        return StoredResult(StoredResult.deserialize(data))
