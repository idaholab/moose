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
from typing import Union
from TestHarness.validation.dataclasses import ValidationResult, ValidationData, ValidationDataTypesStr

# Load the data class module so that we can dynamically instantiate
# validation data types based on their type stored in json
validation_dataclasses_module = importlib.import_module("TestHarness.validation.dataclasses")
for data_type in ValidationDataTypesStr:
    assert hasattr(validation_dataclasses_module, data_type)

NoneType = type(None)

class TestHarnessResults:
    """
    Structure holding the information about a single
    run_tests execution.

    Does not contain the data for each individual test.
    """
    def __init__(self, data: dict):
        # The underlying data from the JSON results, which
        # is the dictionary representation of the entire
        # ".previous_test_results.json" file
        self._data: dict = data

        # Sanity check on all of our methods (in order of definition)
        assert isinstance(self.data, dict)
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
        assert isinstance(self.pr_num, (int, NoneType))
        assert isinstance(self.base_sha, (str, NoneType))
        if self.base_sha:
            assert len(self.base_sha) == 40
        assert isinstance(self.time, datetime)

    @property
    def data(self) -> dict:
        """
        Get the underlying data
        """
        return self._data

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
        return self.civet.get('version', 0)

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
    def pr_num(self) -> Union[int, None]:
        """
        Get the PR number associated with these tests (if any)
        """
        return self.data['pr_num']

    @property
    def base_sha(self) -> Union[str, None]:
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

class TestHarnessTestResult:
    """
    Structure holding the information about a single test result
    """
    def __init__(self, data: dict, result: TestHarnessResults):
        # The underlying data for this test, which comes from
        # "tests/*/tests" in the TestHarness results file
        self._data: dict = data
        # The combined results that this test comes from
        self._results: TestHarnessResults = result

        # Sanity check on all of our data and methods
        assert isinstance(self.data, dict)
        assert isinstance(self.results, TestHarnessResults)
        assert isinstance(self.test_name, str)
        assert isinstance(self.folder_name, str)
        assert isinstance(self.status, dict)
        assert isinstance(self.status_value, str)
        assert isinstance(self.timing, dict)
        assert isinstance(self.run_time, float)
        assert isinstance(self.hpc_queued_time, (float, NoneType))
        assert isinstance(self.event_sha, str)
        assert len(self.event_sha) == 40
        assert isinstance(self.event_cause, str)
        assert isinstance(self.pr_num, (int, NoneType))
        assert isinstance(self.base_sha, (str, NoneType))
        if self.base_sha:
            assert len(self.base_sha) == 40
        assert isinstance(self.time, datetime)
        assert isinstance(self.tester, dict)
        assert isinstance(self.json_metadata, dict)
        assert isinstance(self.hpc, dict)
        assert isinstance(self.hpc_id, (str, NoneType))
        assert isinstance(self.validation, (dict, NoneType))

        # Load the validation results and data
        self._validation_results: list[ValidationResult] = self._buildValidationResults()
        self._validation_data: dict[str, ValidationData] = self._buildValidationData()
        assert isinstance(self.validation_results, list)
        assert isinstance(self.validation_data, dict)

    @property
    def data(self) -> dict:
        """
        Get the underlying data
        """
        return self._data

    @property
    def results(self) -> TestHarnessResults:
        """
        Get the combined results that this test comes from
        """
        return self._results

    @property
    def test_name(self) -> str:
        """
        Get the name of the test
        """
        return self.data['test_name']

    @property
    def folder_name(self) -> str:
        """
        Get the name of the test
        """
        return self.data['folder_name']

    @property
    def status(self) -> dict:
        """
        Get the status entry for the test
        """
        return self.data['status']

    @property
    def status_value(self) -> str:
        """
        Get the status value for the test (OK, ERROR, etc)
        """
        return self.status['status']

    @property
    def timing(self) -> dict:
        """
        Get the timing entry for the test
        """
        return self.data['timing']

    @property
    def run_time(self) -> float:
        """
        Get the run time for this test
        """
        return self.timing['runner_run']

    @property
    def hpc_queued_time(self) -> Union[float, None]:
        """
        Get the time that this test was queued on HPC, if
        it was ran on HPC (otherwise None)
        """
        return self.timing.get('hpc_queued')

    @property
    def event_sha(self) -> str:
        """
        Get the commit that this test was ran on
        """
        assert(self.data['event_sha'] == self.results.event_sha)
        return self.data['event_sha']

    @property
    def event_cause(self) -> str:
        """
        Get the cause for the test that was ran
        """
        assert(self.data['event_cause'] == self.results.event_cause)
        return self.data['event_cause']

    @property
    def pr_num(self) -> Union[int, None]:
        """
        Get the PR number associated with the test (if any)
        """
        assert(self.data['pr_num'] == self.results.pr_num)
        return self.data['pr_num']

    @property
    def base_sha(self) -> Union[str, None]:
        """
        Get the base commit that these tests were ran on
        """
        value = None
        if self.results.civet_version > 1:
            value = self.data['base_sha']
        assert(value == self.results.base_sha)
        return value

    @property
    def time(self) -> datetime:
        """
        Get the time this test was added to the database
        """
        return self.data['time']

    @property
    def tester(self) -> str:
        """
        Get the Tester entry in the data
        """
        return self.data['tester']

    @property
    def json_metadata(self) -> dict:
        """
        Get the 'json_metadata' entry from the Tester entry
        """
        return self.tester.get('json_metadata', {})

    @property
    def hpc(self) -> dict:
        """
        Get the 'hpc' entry if it exists (otherwise, empty dict)
        """
        return self.data.get('hpc', {})

    @property
    def hpc_id(self) -> Union[int, None]:
        """
        Get the HPC job ID that ran this this test, if any
        """
        return self.hpc.get('id')

    @property
    def validation(self) -> dict:
        """
        Get the 'validation' entry for the test, if any
        (otherwise, empty dict)

        Contains the validation data and results
        """
        return self.data.get('validation', {})

    @property
    def validation_data(self) -> dict[str, ValidationData]:
        """
        Get the 'data' entry in 'validation' for this test, if any
        (otherwise, empty dict)
        """
        return self._validation_data

    @property
    def validation_results(self) -> list[ValidationResult]:
        """
        Get the 'data' entry in 'validation' for this test, if any
        (otherwise, empty dict)
        """
        return self._validation_results

    def _buildValidationResults(self) -> list[ValidationResult]:
        """
        Internal method (use on construction) for converting validation
        results in JSON to the underlying ValidationResult objects
        """
        return [ValidationResult(**v) for v in self.validation.get('results', [])]

    def _buildValidationData(self) -> dict[str, ValidationData]:
        """
        Internal method (use on construction) for converting validation
        data in JSON to the underlying ValidationData objects
        """
        input = self.validation.get('data', {})
        data = {}

        for k, v in input.items():
            # Bounds is saved as a list; convert to tuple
            if 'bounds' in v:
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
