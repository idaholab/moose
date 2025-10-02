#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from datetime import datetime
import importlib
from typing import Optional, Union, Iterator
from dataclasses import dataclass
from TestHarness.validation.dataclasses import ValidationResult, ValidationData, ValidationDataTypesStr
from pymongo.database import Database
from bson.objectid import ObjectId

# Load the data class module so that we can dynamically instantiate
# validation data types based on their type stored in json
validation_dataclasses_module = importlib.import_module("TestHarness.validation.dataclasses")
for data_type in ValidationDataTypesStr:
    assert hasattr(validation_dataclasses_module, data_type)

NoneType = type(None)

@dataclass(frozen=True)
class TestName:
    """
    Name for a test (folder and name)
    """
    folder: str
    name: str

    def __str__(self):
        return f'{self.folder}.{self.name}'
class TestHarnessTestResult:
    """
    Structure holding the information about a single test result
    """
    def __init__(self, data: dict, folder_name: str, test_name: str,
                 result: 'TestHarnessResults'):
        # The underlying data for this test, which comes from
        # "tests/*/tests" in the TestHarness results file
        self._data: dict = data
        # The name of the folder this test is in
        self._folder_name: str = folder_name
        # The name of this test
        self._test_name: str = test_name
        # The combined results that this test comes from
        self._results: 'TestHarnessResults' = result

        # Sanity check on all of our data and methods
        assert isinstance(self.data, dict)
        assert isinstance(self.id, ObjectId)
        assert isinstance(self.results, TestHarnessResults)
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
        assert isinstance(self.json_metadata, (dict, NoneType))
        assert isinstance(self.hpc, (dict, NoneType))
        assert isinstance(self.hpc_id, (str, NoneType))
        assert isinstance(self.validation, (dict, NoneType))

        # Load the validation results and data
        self._validation_results: Optional[list[ValidationResult]] = self._buildValidationResults()
        self._validation_data: Optional[dict[str, ValidationData]] = self._buildValidationData()
        assert isinstance(self.validation_results, (list, NoneType))
        assert isinstance(self.validation_data, (dict, NoneType))

    @property
    def data(self) -> dict:
        """
        Get the underlying data
        """
        return self._data

    @property
    def id(self) -> ObjectId:
        """
        Get the mongo database ID for these results
        """
        return self.data['_id']

    @property
    def results(self) -> 'TestHarnessResults':
        """
        Get the combined results that this test comes from
        """
        return self._results

    @property
    def test_name(self) -> str:
        """
        Get the name of the test
        """
        return self._test_name

    @property
    def folder_name(self) -> str:
        """
        Get the name of the folder the test is in
        """
        return self._folder_name

    @property
    def status(self) -> Optional[dict]:
        """
        Get the status entry for the test
        """
        return self.data.get('status')

    @property
    def status_value(self) -> Optional[str]:
        """
        Get the status value for the test (OK, ERROR, etc)
        """
        return self.status['status'] if self.status is not None else None

    @property
    def timing(self) -> Optional[dict]:
        """
        Get the timing entry for the test
        """
        return self.data.get('timing')

    @property
    def run_time(self) -> Optional[float]:
        """
        Get the run time for this test (if available)
        """
        return self.timing.get('runner_run') if self.timing is not None else None

    @property
    def hpc_queued_time(self) -> Optional[float]:
        """
        Get the time that this test was queued on HPC, if
        it was ran on HPC (otherwise None)
        """
        return self.timing.get('hpc_queued') if self.timing is not None else None

    @property
    def event_sha(self) -> str:
        """
        Get the commit that this test was ran on
        """
        return self.results.event_sha

    @property
    def event_cause(self) -> str:
        """
        Get the cause for the test that was ran
        """
        return self.results.event_cause

    @property
    def event_id(self) -> int:
        """
        Get the ID of the event that this job was ran on
        """
        return self.results.event_id

    @property
    def pr_num(self) -> Optional[int]:
        """
        Get the PR number associated with the test (if any)
        """
        return self.results.pr_num

    @property
    def base_sha(self) -> Optional[str]:
        """
        Get the base commit that these tests were ran on
        """
        return self.results.base_sha

    @property
    def time(self) -> datetime:
        """
        Get the time this test was added to the database
        """
        return self.results.time

    @property
    def tester(self) -> Optional[dict]:
        """
        Get the Tester entry in the data
        """
        return self.data.get('tester')

    @property
    def json_metadata(self) -> Optional[dict]:
        """
        Get the 'json_metadata' entry from the Tester entry
        """
        return self.tester.get('json_metadata') if self.tester is not None else None

    @property
    def hpc(self) -> Optional[dict]:
        """
        Get the 'hpc' entry if it exists
        """
        return self.data.get('hpc')

    @property
    def hpc_id(self) -> Optional[int]:
        """
        Get the HPC job ID that ran this this test, if any
        """
        return self.hpc.get('id') if self.hpc is not None else None

    @property
    def validation(self) -> Optional[dict]:
        """
        Get the 'validation' entry for the test, if any

        Contains the validation data and results
        """
        return self.data.get('validation')

    @property
    def validation_data(self) -> Optional[dict[str, ValidationData]]:
        """
        Get the 'data' entry in 'validation' for this test, if any
        """
        return self._validation_data

    @property
    def validation_results(self) -> Optional[list[ValidationResult]]:
        """
        Get the 'data' entry in 'validation' for this test, if any
        (otherwise, empty dict)
        """
        return self._validation_results

    def _buildValidationResults(self) -> Optional[list[ValidationResult]]:
        """
        Internal method (use on construction) for converting validation
        results in JSON to the underlying ValidationResult objects
        """
        if self.validation is not None:
            return [ValidationResult(**v) for v in self.validation.get('results', [])]
        return None

    def _buildValidationData(self) -> Optional[dict[str, ValidationData]]:
        """
        Internal method (use on construction) for converting validation
        data in JSON to the underlying ValidationData objects
        """
        if self.validation is None:
            return None

        input = self.validation.get('data', {})
        data = {}

        for k, v in input.items():
            # Bounds is saved as a list; convert to tuple
            if v.get('bounds') is not None:
                v['bounds'] = tuple(v['bounds'])

            # Before version 1, the data type was not stored
            if self.results.validation_version == 0:
                data_type = 'ValidationScalarData'
            # After version 1, the type is explicitly available
            else:
                data_type = v.pop('type')
            if data_type not in ValidationDataTypesStr:
                raise ValueError(f"Unknown validation data type {data_type} for data '{k}'")

            # Build the underlying data class instead of a dict
            data[k] = getattr(validation_dataclasses_module, data_type)(**v)

        return data

class TestHarnessResults:
    """
    Structure holding the information about a single
    run_tests execution.

    Does not contain the data for each individual test.
    """
    def __init__(self, data: dict, db: Optional[Database] = None):
        # The underlying data from the JSON results, which
        # is the dictionary representation of the entire
        # ".previous_test_results.json" file
        assert isinstance(data, dict)
        self._data: dict = data

        # The database connection, used for querying test results
        assert isinstance(db, (Database, NoneType))
        self._db: Optional[Database] = db

        # Initialize empty test objects
        tests = data['tests']
        assert isinstance(tests, dict)
        self._tests = self.build_empty_tests(tests)

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
        assert isinstance(self.test_names, list)
        for v in self.test_names:
            assert isinstance(v, TestName)

    @staticmethod
    def build_empty_tests(tests: dict) -> dict[TestName, Union[TestHarnessTestResult, ObjectId]]:
        """
        Builds the names (folder, name) of the held tests
        """
        assert isinstance(tests, dict)

        values: dict[TestName, ObjectId] = {}
        for test_folder, folder_entry in tests.items():
            for test_name, id in folder_entry['tests'].items():
                assert id not in values.values()
                values[TestName(test_folder, test_name)] = id

        return values

    @property
    def data(self) -> dict:
        """
        Get the underlying data
        """
        return self._data

    @property
    def id(self) -> ObjectId:
        """
        Get the mongo database ID for these results
        """
        return self.data['_id']

    @property
    def testharness(self) -> dict:
        """
        Get the testharness entry in the data
        """
        return self.data['testharness']

    @property
    def version(self) -> int:
        """
        Get the result version
        """
        return self.testharness['version']

    @property
    def validation_version(self) -> int:
        """
        Get the validation version
        """
        return self.testharness.get('validation_version', 0)

    @property
    def civet(self) -> dict:
        """
        Get the CIVET entry from the data, which contains
        information about the CIVET job that ran this test
        """
        return self.data['civet']

    @property
    def civet_version(self) -> int:
        """
        Get the CIVET schema version
        """
        # Version was first added to ['civet']['version'], and
        # was then moved to ['civet_version']
        version = self.civet.get('version', self.data.get('civet_version', 0))
        assert isinstance(version, int)
        return version

    @property
    def civet_job_url(self) -> str:
        """
        Get the URL to the CIVET job that ran this test
        """
        return self.civet['job_url']

    @property
    def civet_job_id(self) -> int:
        """
        Get the ID of the civet job that ran this test
        """
        return self.civet['job_id']

    @property
    def hpc(self) -> dict:
        """
        Get the HPC entry that describes the HPC environment
        these tests ran on, if any (empty dict if not)
        """
        return self.data.get('hpc', {})

    @property
    def event_sha(self) -> str:
        """
        Get the commit that these tests were ran on
        """
        return self.data['event_sha']

    @property
    def event_cause(self) -> str:
        """
        Get the cause for these tests that were ran
        """
        return self.data['event_cause']

    @property
    def event_id(self) -> int:
        """
        Get the ID of the civet event that ran this test
        """
        if self.civet_version > 2:
            id = self.data['event_id']
            assert isinstance(id, int)
            return id
        return None

    @property
    def pr_num(self) -> Optional[int]:
        """
        Get the PR number associated with these tests (if any)
        """
        return self.data['pr_num']

    @property
    def base_sha(self) -> Optional[str]:
        """
        Get the base commit that these tests were ran on
        """
        if self.civet_version < 2:
            return None
        return self.data['base_sha']

    @property
    def time(self) -> datetime:
        """
        Get the time these tests were added to the database
        """
        return self.data['time']

    @property
    def test_names(self) -> list[TestName]:
        """
        Get the combined names of all tests
        """
        return list(self._tests.keys())

    def has_test(self, folder_name: str, test_name: str) -> bool:
        """
        Whether or not a test with the given folder and test name is stored
        """
        assert isinstance(folder_name, str)
        assert isinstance(test_name, str)

        key = TestName(folder_name, test_name)
        return key in self._tests

    def _find_test_data(self, id: ObjectId) -> dict:
        """
        Helper for getting the data associated with a test

        This is a separate function so that it can be mocked
        easily within unit tests
        """
        assert self._db is not None
        assert isinstance(self._db, Database)
        assert isinstance(id, ObjectId)

        return self._db.tests.find_one({"_id": id})

    def get_test(self, folder_name: str, test_name: str) -> TestHarnessTestResult:
        """
        Get the test result associated with the given test folder and test name
        """
        assert isinstance(folder_name, str)
        assert isinstance(test_name, str)

        # Search for the data in the cache
        key = TestName(folder_name, test_name)
        value = self._tests.get(key)

        # Doesn't exist
        if value is None:
            raise KeyError(f'No test named {key}')

        # Is already built
        if isinstance(value, TestHarnessTestResult):
            return value

        # Not built, so pull from database
        data = self._find_test_data(value)
        if data is None:
            raise KeyError(f'Database missing results: _id={value}')

        # Build the true object from the data
        try:
            test_result = TestHarnessTestResult(data, folder_name, test_name, self)
        except Exception as e:
            raise Exception(f'Failed to build result: _id={value}') from e

        # And store in the cache
        self._tests[key] = test_result

        return test_result

    def get_tests(self) -> Iterator[TestHarnessTestResult]:
        """
        Get all of the test results
        """
        for combined_name in self._tests:
            yield self.get_test(combined_name.folder, combined_name.name)
