# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.storedresults."""

import unittest
from copy import deepcopy
from dataclasses import dataclass
from typing import Optional

from bson.objectid import ObjectId
from mock import patch
from test_resultsstore_civetstore import build_civet_env

from TestHarness import TestHarness
from TestHarness.resultsstore.civetstore import CIVETStore
from TestHarness.resultsstore.storedresults import (
    DatabaseException,
    StoredResult,
    StoredTestResult,
)
from TestHarness.resultsstore.utils import (
    TestName,
    compress_dict,
    decompress_dict,
    results_set_test_value,
    results_test_entry,
    results_test_iterator,
)
from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase

# ID for the tested built StoredResult object
RESULT_ID = ObjectId()
# A dummy environment from CIVET to build a dummy header
BASE_SHA, CIVET_ENV = build_civet_env()
# A dummy header (entries added to the result entry)
STORE_HEADER = CIVETStore.build_header(BASE_SHA, CIVET_ENV)
# Dummy value for a test's ['timing']['hpc_queued_time']
HPC_QUEUED_TIME = 1.234


class TestResultsStoredResults(TestHarnessTestCase):
    """
    Test the StoredResult and StoredTestResult.

    We will not interact with a database at all here, we will start
    with TestHarness JSON output and:
        - Add the header (built with CIVETStore) that contains the
            indices (base sha, head sha, event id, etc...)
        - Remove all output, which CIVETStore does
        - Faked IDs for the entries in the database
        - Compress the test ['tester']['json_metadata'] entries,
            as CIVETStore would do

    We will also test for capabilities that are changed over time
    as the CIVETStore.CIVET_VERSION changes, which denotes changes
    in the structure of the data.
    """

    @dataclass
    class ConvertedTestHarnessResults:
        """Helper data class for convertTestHarnessResults()."""

        # The data for building the StoredResult
        result_data: dict
        # The data for the tests, indexed by TestName
        test_data: dict[TestName, dict]

    @staticmethod
    def convertTestHarnessResults(
        test_harness_results: dict,
        remove_header: list = [],
        civet_version: int = CIVETStore.CIVET_VERSION,
        test_in_results: bool = True,
        no_tests: bool = False,
        delete_test_key: Optional[list[str]] = None,
    ) -> ConvertedTestHarnessResults:
        """
        Take the passed in results and build data used to build objects.

        This exists as a static method so that other tests can
        use it to generate their own data.

        Parameters
        ----------
        test_harness_results : dict
            The results that come from TestHarness JSON result output.

        Optional Parameters
        -------------------
        remove_header : dict
            Remove these keys from the header
        civet_version : int
            Modify the civet_version key
        test_in_results : bool
            True to put the tests in with the results, False to
            store them separately
        no_tests : bool
            Don't store any tests at all
        delete_test_key : list
            Delete this key from each test entry (can be nested)

        Returns
        -------
        ConvertedTestHarnessResults:
            Structure that contains the dict to build the StoredResult
            and optionally the separate test data

        """
        # Modify each test
        tests = {}
        for test in results_test_iterator(test_harness_results):
            name = test.name
            test_values = test.value

            # Test would have an ID in the database
            test_values["_id"] = ObjectId()
            # And should reference a result in the database
            test_values["result_id"] = RESULT_ID

            # Remove output entires, as they're removed when storing
            for key in ["output", "output_files"]:
                if key in test_values:
                    del test_values[key]

            # Compress JSON metadata, which should only exist
            # if the test actually ran (we run this with
            # --capture-perf-graph, so a 'perf_graph' entry
            # will exist here)
            json_metadata = test_values["tester"].get("json_metadata")
            if json_metadata is not None:
                for k, v in json_metadata.items():
                    json_metadata[k] = compress_dict(v)

            # Fake values from HPC
            test_values["timing"]["hpc_queued"] = HPC_QUEUED_TIME

            # Delete requested key
            if delete_test_key:
                current = test_values
                for key in delete_test_key[:-1]:
                    current = current[key]
                del current[delete_test_key[-1]]

            tests[name] = test_values

        # Setup main results entry
        results = test_harness_results.copy()
        # Dummy ID, which is the same set as 'result_id' in the tests
        results["_id"] = RESULT_ID

        # Setup the tests entry, if not instructed not to do so
        results["tests"] = {}
        if not no_tests:
            for name, values in tests.items():
                if name.folder not in results["tests"]:
                    results["tests"][name.folder] = {"tests": {}}
                set_value = values.copy() if test_in_results else values["_id"]
                results_set_test_value(results, name, set_value)

        # Add in header, removing keys to be deleted
        header = deepcopy(STORE_HEADER)
        header["civet_version"] = civet_version
        for k in remove_header:
            del header[k]
        results.update(header)

        return TestResultsStoredResults.ConvertedTestHarnessResults(
            result_data=results, test_data=tests
        )

    @dataclass
    class CapturedResult(ConvertedTestHarnessResults):
        """Helper data class for captureResult()."""

        # The test harness from the run
        harness: TestHarness

    def captureResult(self, **kwargs) -> CapturedResult:
        """Build TestHarness JSON output and produce data used for testing."""
        result = self.runTestsCached(
            "-i", "validation", "--capture-perf-graph", exit_code=132
        )

        # For pylance
        assert isinstance(result.results, dict)
        assert isinstance(result.harness, TestHarness)

        converted = self.convertTestHarnessResults(result.results, **kwargs)
        return self.CapturedResult(
            result_data=converted.result_data,
            test_data=converted.test_data,
            harness=result.harness,
        )

    @dataclass
    class BuiltResult:
        """Helper data class for buildResult()."""

        # The test harness from the run
        harness: TestHarness
        # The built StoredResult object
        results: StoredResult
        # The test data
        test_data: dict

    def buildResult(self, *args, **kwargs) -> BuiltResult:
        """Similar to captureResult(), but also builds a StoredResult."""
        captured_result = self.captureResult(*args, **kwargs)

        results = StoredResult(captured_result.result_data)
        if not kwargs.get("no_tests", False):
            self.assertEqual(results.get_num_tests(), len(captured_result.test_data))

        return self.BuiltResult(
            harness=captured_result.harness,
            results=results,
            test_data=captured_result.test_data,
        )

    def testStoredResult(self):
        """Test a basic build of a StoredResult and the underlying test data."""
        built_result = self.buildResult()
        harness = built_result.harness
        results = built_result.results

        self.assertEqual(results.data, results._data)
        self.assertEqual(results.id, RESULT_ID)
        self.assertEqual(results.testharness, results.data["testharness"])
        self.assertEqual(results.version, results.data["testharness"]["version"])
        self.assertEqual(
            results.validation_version,
            results.data["testharness"]["validation_version"],
        )

        # Faked entries from the CIVET header
        self.assertEqual(results.civet_job_url, STORE_HEADER["civet"]["job_url"])
        self.assertEqual(results.civet_job_id, STORE_HEADER["civet"]["job_id"])
        self.assertEqual(results.civet_version, CIVETStore.CIVET_VERSION)
        self.assertEqual(results.event_sha, STORE_HEADER["event_sha"])
        self.assertEqual(results.event_cause, STORE_HEADER["event_cause"])
        self.assertEqual(results.event_id, STORE_HEADER["event_id"])
        self.assertEqual(results.pr_num, STORE_HEADER["pr_num"])
        self.assertEqual(results.base_sha, BASE_SHA)
        self.assertEqual(results.time, STORE_HEADER["time"])

        # Has same number of tests
        self.assertEqual(
            results.get_num_tests(),
            len(list(results_test_iterator(results.data))),
        )
        for test_result in results.get_tests():
            data = built_result.test_data[test_result.name]

            # Get the actual Job object from the TestHarness
            jobs = [
                j
                for j in harness.finished_jobs
                if j.getTestName() == str(test_result.name)
            ]
            self.assertEqual(len(jobs), 1)
            job = jobs[0]

            # Base properties
            self.assertEqual(test_result.data, data)
            self.assertEqual(test_result.id, data["_id"])
            self.assertEqual(test_result.results, results)
            self.assertEqual(test_result.result_id, results.id)
            self.assertEqual(test_result.folder_name, test_result.name.folder)
            self.assertEqual(test_result.test_name, test_result.name.name)

            # Status properties
            self.assertEqual(test_result.status, data["status"])
            self.assertEqual(test_result.status_value, data["status"]["status"])

            # Timing properties
            self.assertEqual(test_result.timing, data["timing"])
            self.assertEqual(test_result.run_time, data["timing"]["runner_run"])
            self.assertEqual(test_result.hpc_queued_time, HPC_QUEUED_TIME)

            # Faked entries from the CIVET header
            self.assertEqual(test_result.event_sha, STORE_HEADER["event_sha"])
            self.assertEqual(test_result.event_cause, STORE_HEADER["event_cause"])
            self.assertEqual(test_result.event_id, STORE_HEADER["event_id"])
            self.assertEqual(test_result.pr_num, STORE_HEADER["pr_num"])
            self.assertEqual(test_result.base_sha, BASE_SHA)
            self.assertEqual(test_result.time, STORE_HEADER["time"])

            # Tester properties
            self.assertEqual(test_result.tester, data["tester"])

            # JSON metadata, which only appears for tests that ran
            if test_result.status_value == "OK":
                # Decompress the data and compare
                json_metadata = data["tester"]["json_metadata"]
                if json_metadata is not None:
                    self.assertEqual(len(json_metadata), len(test_result.json_metadata))
                    for k, v in json_metadata.items():
                        self.assertIn(k, test_result.json_metadata)
                        decompressed = decompress_dict(v)
                        self.assertEqual(test_result.json_metadata[k], decompressed)

                # Should thus have a perf_graph entry
                self.assertEqual(
                    test_result.perf_graph, decompress_dict(json_metadata["perf_graph"])
                )
            # Test didn't run, so no metadata or perf_graph
            else:
                self.assertEqual(len(test_result.json_metadata), 0)
                self.assertIsNone(test_result.perf_graph)

            # Validation data
            results_i = 0
            for case in job.validation_cases:
                # Results equivalent to the actual Job Result objects
                for v in case.results:
                    self.assertEqual(v, test_result.validation_results[results_i])
                    results_i += 1

                # Validation data is equivalent to the actual Data objects
                for k, v in case.data.items():
                    self.assertEqual(v, test_result.validation_data[k])

    def testStoredResultNoCheck(self):
        """Test building a StoredResult with check=False."""

    @patch.object(StoredResult, "_find_test_data")
    def testGetTest(self, patch_find_test_data):
        """Tests when tests are stored as ObjectId and then loaded."""
        captured_result = self.captureResult(test_in_results=False)
        test_data = captured_result.test_data
        result_data = captured_result.result_data

        # Mock getting the test results from mongodb
        patch_find_test_data.side_effect = lambda id: next(
            (v for v in test_data.values() if v["_id"] == id), None
        )

        result = StoredResult(result_data)
        self.assertEqual(result.get_num_tests(), len(test_data))

        for name, data in test_data.items():
            self.assertTrue(result.has_test(name))
            self.assertIsInstance(results_test_entry(result.data, name), ObjectId)

            # Loads from cache
            test_result = result.get_test(name)
            self.assertEqual(result._tests[name], test_result)

            # Exists in cache
            self.assertEqual(test_result, result.get_test(name))

    def testGetTestNotStored(self):
        """Test TestHarnessResult.[have_test, get_test]() on a non-stored test."""
        built_result = self.buildResult()
        results = built_result.results

        name = TestName("foo", "bar")
        self.assertFalse(results.has_test(name))
        with self.assertRaisesRegex(KeyError, f'Test "{name}" does not exist'):
            results.get_test(name)

    @patch.object(StoredResult, "_find_test_data")
    def testGetTestMissingDatabase(self, patch_find_test_data):
        """Tests TestHarnessResult.get_test() with a missing test in the database."""
        patch_find_test_data.side_effect = lambda id: None

        built_result = self.buildResult(test_in_results=False)
        results = built_result.results

        for name in results.get_test_names():
            id = results_test_entry(results.data, name)
            with self.assertRaisesRegex(
                DatabaseException, f"Database missing tests._id={id}"
            ):
                results.get_test(name)

    @patch.object(StoredTestResult, "__init__")
    def testBuildTestException(self, patch_init):
        """Tests TestHarnessResult._build_test() raising when the build fails."""
        patch_init.side_effect = Exception("foo")

        with self.assertRaisesRegex(ValueError, "Failed to build test result"):
            built_result = self.buildResult()
            built_result.results.load_all_tests()

    def testDeprecatedBaseSHA(self):
        """Test when base_sha didn't exist in the database (civet version < 2)."""
        civet_version = 1
        built_result = self.buildResult(
            remove_header=["base_sha"], civet_version=civet_version, no_tests=True
        )
        self.assertEqual(built_result.results.civet_version, civet_version)
        self.assertIsNone(built_result.results.base_sha)

    def testNoEventID(self):
        """Test when event_id didn't exist in the database (civet_version < 3)."""
        civet_version = 2
        built_result = self.buildResult(
            remove_header=["event_id"], civet_version=civet_version, no_tests=True
        )
        self.assertEqual(built_result.results.civet_version, civet_version)
        self.assertIsNone(built_result.results.event_id)

    def testNoTiming(self):
        """Test timing not being available in a test result."""
        built_result = self.buildResult(delete_test_key=["timing"])
        for test_result in built_result.results.get_tests():
            self.assertIsNone(test_result.timing)
            self.assertIsNone(test_result.run_time)
            self.assertIsNone(test_result.hpc_queued_time)

    def testNoRunTime(self):
        """Test run_time not being available in timing in a test result."""
        built_result = self.buildResult(delete_test_key=["timing", "runner_run"])
        for test_result in built_result.results.get_tests():
            self.assertIsNone(test_result.run_time)

    def testNoHPCQueuedTime(self):
        """Test hpc_queued_time not being available in timing in a test result."""
        built_result = self.buildResult(delete_test_key=["timing", "hpc_queued"])
        for test_result in built_result.results.get_tests():
            self.assertIsNone(test_result.hpc_queued_time)

    def testNoTester(self):
        """Test tester not being available in a test result."""
        built_result = self.buildResult(delete_test_key=["tester"])
        for test_result in built_result.results.get_tests():
            self.assertIsNone(test_result.tester)
            self.assertEqual(len(test_result.json_metadata), 0)

    def testNoStatus(self):
        """Test status not being available in a test result."""
        built_result = self.buildResult(delete_test_key=["status"])
        for test_result in built_result.results.get_tests():
            self.assertIsNone(test_result.status)
            self.assertIsNone(test_result.status_value)

    def testNoTestResultID(self):
        """Test result_id not being within a test."""
        built_result = self.buildResult(delete_test_key=["result_id"])
        for test_result in built_result.results.get_tests():
            self.assertEqual(test_result.result_id, built_result.results.id)

    def _compareSerialized(
        self, result: StoredResult, new_result: StoredResult, same_tests: bool
    ):
        # Test keys should be the same (values will not be
        # because they were loaded from the database and
        # were originally stored as IDs)
        self.assertEqual(result.data["tests"].keys(), new_result.data["tests"].keys())
        # And the underlying test objects should also be the same
        new_result.load_all_tests()
        for test_name in result.get_test_names():
            test = result._tests[test_name]
            assert isinstance(test, StoredTestResult)  # for pylance

            new_test = new_result._tests[test_name]
            assert isinstance(new_test, StoredTestResult)  # for pylance
            self.assertEqual(test._data, new_test._data)
            self.assertEqual(test.name, new_test.name)

            self.assertEqual(test._validation_results, new_test._validation_results)
            self.assertEqual(test._validation_data, new_test._validation_data)

            self.assertEqual(test.json_metadata, new_test.json_metadata)
            self.assertEqual(test.perf_graph, new_test.perf_graph)

        # Compare all of the data
        new_data = deepcopy(new_result.data)
        # If the tests entry isn't exactly the same, just
        # duplicate it
        if not same_tests:
            new_data["tests"] = deepcopy(result.data["tests"])
        self.assertEqual(result.data, new_data)

    @patch.object(StoredResult, "_find_tests_data")
    def testSerializeTestsNotLoaded(self, patch_find_tests_data):
        """Test serializing and deserializing a result when the data is not loaded."""
        captured_result = self.captureResult(test_in_results=False)
        result_data = captured_result.result_data
        test_data = captured_result.test_data

        # Mock getting the test results from mongodb
        def find_tests_data(ids):
            return [v for v in test_data.values() if v["_id"] in ids]

        patch_find_tests_data.side_effect = find_tests_data

        result = StoredResult(result_data)

        # Tests should not be loaded yet
        test_names = result.get_test_names()
        self.assertTrue(test_names)
        for name in test_names:
            self.assertNotIn(name, result._tests)

        serialized = result.serialize()

        # And should be loaded now
        for entry in result._tests.values():
            self.assertIsInstance(entry, StoredTestResult)

        new_result = StoredResult.deserialize_build(serialized)

        # Compare all of the data
        self._compareSerialized(result, new_result, False)

    def testSerializeTestsInResult(self):
        """Test serializing and deserializing not loading from the database."""
        built_result = self.buildResult()
        results = built_result.results

        # Load tests
        self.assertEqual(0, len(results._tests))
        num_tests = results.get_num_tests()
        results.load_all_tests()
        self.assertEqual(num_tests, len(results._tests))

        serialized = results.serialize()

        new_result = StoredResult.deserialize_build(serialized)

        # Compare all of the data
        self._compareSerialized(results, new_result, True)

    def testSerializeWithFilter(self):
        """Test serialize() with test_filter set, only storing specific tests."""
        results = self.buildResult().results

        tests = list(results_test_iterator(results.data))
        self.assertGreater(len(tests), 1)
        keep_test = tests[0].name

        serialized = results.serialize(test_filter=[keep_test])
        serialized_tests = list(results_test_iterator(serialized))
        self.assertEqual(len(serialized_tests), 1)
        self.assertEqual(serialized_tests[0].name, keep_test)

    def testSerializeLoadAllTests(self):
        """Test that serialize() loads all tests."""
        built_result = self.buildResult()
        results = built_result.results
        results.serialize()
        self.assertTrue(results._all_tests_loaded)

    @patch.object(StoredResult, "_find_tests_data")
    def testLoadAllTestsMissing(self, patch_find_tests_data):
        """Test StoredResult.load_all_data when test(s) are not found."""
        captured_result = self.captureResult(test_in_results=False)

        patch_find_tests_data.return_value = []

        with self.assertRaisesRegex(KeyError, "Failed to load test results"):
            StoredResult(captured_result.result_data).load_all_tests()

    def testRecreateResult(self):
        """
        Test recreating a result with just the data.

        This enables caching of results in web applications.
        """
        result = self.buildResult().results
        new_result = StoredResult(deepcopy(result.data))
        self.assertEqual(result.data, new_result.data)

    # def testRecreateResultNoCheck(self):
    #     """
    #     Test recreating a result with just the data.

    #     This enables caching of results in web applications.
    #     """
    #     result = self.buildResult().results
    #     new_result = StoredResult(deepcopy(result.data))
    #     self.assertEqual(result.data, new_result.data)


if __name__ == "__main__":
    unittest.main()
