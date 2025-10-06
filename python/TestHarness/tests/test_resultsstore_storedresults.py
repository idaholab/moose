#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable
import unittest
import json
import zlib
from bson.objectid import ObjectId
from datetime import datetime
from mock import patch
from copy import deepcopy
from typing import Tuple
from dataclasses import dataclass
from TestHarness import TestHarness
from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase
from TestHarness.resultsstore.storedresults import StoredResult, StoredTestResult, TestName, DatabaseException
from TestHarness.resultsstore.civetstore import compress_dict, decompress_dict

FAKE_RESULT_ID = ObjectId()
FAKE_CIVET_VERSION = 4
FAKE_CIVET_JOB_ID = 12345
FAKE_CIVET_JOB_URL = f'civet.inl.gov/job/{FAKE_CIVET_JOB_ID}'
FAKE_EVENT_SHA = 'abcd1234ababcd1234ababcd1234ababcd1234ab'
FAKE_BASE_SHA = '1234abcdab1234abcdab1234abcdab1234abcdab'
FAKE_EVENT_CAUSE = 'pr'
FAKE_EVENT_ID = 5678
FAKE_PR_NUM = 1234
FAKE_HPC_QUEUED_TIME = 1.234
FAKE_TIME = datetime.now()
FAKE_TEST_NAME = TestName('folder', 'name')

class TestResultsStoredResults(TestHarnessTestCase):
    @dataclass
    class CapturedResult:
        """
        Helper data class for captureResult()
        """
        # The test harness from the run
        harness: TestHarness
        # The data for building the StoredResult
        result_data: dict
        # The data for building the tests
        test_data: dict

    def captureResult(self, remove_civet_keys=[], civet_version=FAKE_CIVET_VERSION,
                      test_in_results=True, no_tests=False, delete_test_key=None) -> CapturedResult:
        result = self.runTestsCached('-i', 'validation', '--capture-perf-graph', exit_code=132)
        values = result.results

        tests = {}

        # Faked values that CIVET would add
        civet_values = {'event_sha': FAKE_EVENT_SHA,
                        'event_cause': FAKE_EVENT_CAUSE,
                        'event_id': FAKE_EVENT_ID,
                        'pr_num': FAKE_PR_NUM,
                        'base_sha': FAKE_BASE_SHA,
                        'time': FAKE_TIME}
        for k in remove_civet_keys:
            del civet_values[k]

        # Perform fixups that CIVET would do
        for folder_name, folder_values in values['tests'].items():
            for test_name, test_values in folder_values['tests'].items():
                name = TestName(folder_name, test_name)
                test_values['_id'] = ObjectId()

                # Remove output entires, as they're removed when storing
                for key in ['output', 'output_files']:
                    if key in test_values:
                        del test_values[key]

                # Compress JSON metadata, which should only exist
                # if the test actually ran
                json_metadata = test_values['tester'].get('json_metadata')
                if test_values['status']['status'] == 'OK':
                    for k, v in json_metadata.items():
                        json_metadata[k] = compress_dict(v)
                else:
                    self.assertIsNone(json_metadata)

                # Fake values from HPC
                test_values['timing']['hpc_queued'] = FAKE_HPC_QUEUED_TIME
                # Fake result ID
                test_values['result_id'] = FAKE_RESULT_ID

                # Delete requested key
                if delete_test_key:
                    current = test_values
                    for key in delete_test_key[:-1]:
                        current = current[key]
                    del current[delete_test_key[-1]]

                tests[name] = test_values

        # Setup main results entry
        results = values.copy()
        # Dummy ID
        results['_id'] = FAKE_RESULT_ID
        # Tests entry
        results['tests'] = {}
        if not no_tests:
            for name, values in tests.items():
                if name.folder not in results['tests']:
                    results['tests'][name.folder] = {'tests': {}}
                if test_in_results:
                    results['tests'][name.folder]['tests'][name.name] = values.copy()
                else:
                    results['tests'][name.folder]['tests'][name.name] = values['_id']
        # CIVET key
        results['civet'] = {'job_url': FAKE_CIVET_JOB_URL,
                            'job_id': FAKE_CIVET_JOB_ID,
                            'version': civet_version}
        # Base CIVET values for indexing
        results.update(civet_values)

        return self.CapturedResult(harness=result.harness, result_data=results, test_data=tests)

    @dataclass
    class BuiltResult:
        """
        Helper data class for buildResult()
        """
        # The test harness from the run
        harness: TestHarness
        # The built StoredResult object
        results: StoredResult
        # The test data
        test_data: dict

    def buildResult(self, *args, **kwargs) -> BuiltResult:
        captured_result = self.captureResult(*args, **kwargs)

        results = StoredResult(captured_result.result_data)
        if not kwargs.get('no_tests', False):
            self.assertEqual(len(results._tests), len(captured_result.test_data))

        return self.BuiltResult(harness=captured_result.harness, results=results, test_data=captured_result.test_data)

    def testTestHarnessResult(self):
        """
        Tests a basic build of a TestHarnessResult and the underlying test data
        """
        built_result = self.buildResult()
        harness = built_result.harness
        results = built_result.results

        self.assertEqual(results.data, results._data)
        self.assertEqual(results.id, FAKE_RESULT_ID)
        self.assertEqual(results.testharness, results.data['testharness'])
        self.assertEqual(results.version, results.data['testharness']['version'])
        self.assertEqual(results.validation_version, results.data['testharness']['validation_version'])

        # Faked entries for civet
        self.assertEqual(results.civet_job_url, FAKE_CIVET_JOB_URL)
        self.assertEqual(results.civet_job_id, FAKE_CIVET_JOB_ID)
        self.assertEqual(results.civet_version, FAKE_CIVET_VERSION)
        self.assertEqual(results.event_sha, FAKE_EVENT_SHA)
        self.assertEqual(results.event_cause, FAKE_EVENT_CAUSE)
        self.assertEqual(results.event_id, FAKE_EVENT_ID)
        self.assertEqual(results.pr_num, FAKE_PR_NUM)
        self.assertEqual(results.base_sha, FAKE_BASE_SHA)
        self.assertEqual(results.time, FAKE_TIME)

        # Has same number of tests
        self.assertEqual(results.num_tests, len(results._tests))
        self.assertEqual(results.num_tests, len(results.tests))
        for test_result in results.tests:
            data = built_result.test_data[test_result.name]

            # Get the actual Job object from the TestHarness
            jobs = [j for j in harness.finished_jobs if j.getTestName() == str(test_result.name)]
            self.assertEqual(len(jobs), 1)
            job = jobs[0]

            # Base properties
            self.assertEqual(test_result.data, data)
            self.assertEqual(test_result.id, data['_id'])
            self.assertEqual(test_result.results, results)
            self.assertEqual(test_result.result_id, results.id)
            self.assertEqual(test_result.folder_name, test_result.name.folder)
            self.assertEqual(test_result.test_name, test_result.name.name)

            # Status properties
            self.assertEqual(test_result.status, data['status'])
            self.assertEqual(test_result.status_value, data['status']['status'])

            # Timing properties
            self.assertEqual(test_result.timing, data['timing'])
            self.assertEqual(test_result.run_time, data['timing']['runner_run'])
            self.assertEqual(test_result.hpc_queued_time, FAKE_HPC_QUEUED_TIME)

            # Faked CIVET properties
            self.assertEqual(test_result.event_sha, FAKE_EVENT_SHA)
            self.assertEqual(test_result.event_cause, FAKE_EVENT_CAUSE)
            self.assertEqual(test_result.event_id, FAKE_EVENT_ID)
            self.assertEqual(test_result.pr_num, FAKE_PR_NUM)
            self.assertEqual(test_result.base_sha, FAKE_BASE_SHA)
            self.assertEqual(test_result.time, FAKE_TIME)

            # Tester properties
            self.assertEqual(test_result.tester, data['tester'])

            # JSON metadata, which only appears for tests that ran
            if test_result.status_value == 'OK':
                # Decompress the data and compare
                json_metadata = data['tester']['json_metadata']
                if json_metadata is not None:
                    self.assertEqual(len(json_metadata), len(test_result.json_metadata))
                    for k, v in json_metadata.items():
                        self.assertIn(k, test_result.json_metadata)
                        decompressed = decompress_dict(v)
                        self.assertEqual(test_result.json_metadata[k], decompressed)

                # Should thus have a perf_graph entry
                self.assertEqual(test_result.perf_graph, decompress_dict(json_metadata['perf_graph']))
            # Test didn't run, so no metadata or perf_graph
            else:
                self.assertIsNone(test_result.json_metadata)
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

    @patch.object(StoredResult, '_find_test_data')
    def testGetTest(self, patch_find_test_data):
        """
        Tests when tests are stored as ObjectId (reference to another collection)
        and then later loaded from the database on request
        """
        captured_result = self.captureResult(test_in_results=False)
        test_data = captured_result.test_data
        result_data = captured_result.result_data

        # Mock getting the test results from mongodb
        def get_test_data(id):
            for test in test_data.values():
                if test['_id'] == id:
                    return test
            return None
        patch_find_test_data.side_effect = get_test_data

        result = StoredResult(result_data)
        self.assertEqual(len(result._tests), len(test_data))

        for name, data in test_data.items():
            self.assertTrue(result.has_test(name.folder, name.name))

            # Not loaded from cache
            id = data['_id']
            self.assertIn(name, result._tests)
            self.assertEqual(result._tests[name], id)

            # Loads from cache
            test_result = result.get_test(name.folder, name.name)
            self.assertEqual(result._tests[name], test_result)

            # Exists in cache
            self.assertEqual(test_result, result.get_test(name.folder, name.name))

    def testGetTestNotStored(self):
        """
        Tests TestHarnessResult.[have_test, get_test] on a test
        that is not stored
        """
        built_result = self.buildResult()
        results = built_result.results

        self.assertFalse(results.has_test('foo', 'bar'))
        with self.assertRaisesRegex(KeyError, 'Test "foo.bar" does not exist'):
            results.get_test('foo', 'bar')

    @patch.object(StoredResult, '_find_test_data')
    def testGetTestMissingDatabase(self, patch_find_test_data):
        """
        Tests TestHarnessResult.get_test when a test does not exist
        in the database
        """
        patch_find_test_data.side_effect = lambda id: None

        built_result = self.buildResult(test_in_results=False)
        results = built_result.results

        for name in results.test_names:
            id = results._tests[name]
            with self.assertRaisesRegex(DatabaseException, f'Database missing tests._id={id}'):
                results.get_test(name.folder, name.name)

    @patch.object(StoredTestResult, '__init__')
    def testBuildTestException(self, patch_init):
        """
        Tests TestHarnessResult._build_test throwing an excption with the ID
        when the build fails
        """
        patch_init.side_effect = Exception('foo')

        with self.assertRaisesRegex(ValueError, 'Failed to build test result'):
            self.buildResult()

    def testInitTestsBadType(self):
        """
        Tests TestHarnessResult.init_tests when a test entry has a bad type
        (not ObjectID or dict)
        """
        captured_result = self.captureResult()
        result_data = captured_result.result_data

        for folder_name, folder_entry in result_data['tests'].items():
            for test_name in list(folder_entry['tests'].keys()):
                folder_entry['tests'][test_name] = 1

        with self.assertRaisesRegex(TypeError, 'has unexpected type "int"'):
            StoredResult(result_data)

    def testDeprecatedBaseSHA(self):
        """
        Tests when base_sha didn't exist in the database (civet version < 2)
        """
        civet_version = 1
        built_result = self.buildResult(remove_civet_keys=['base_sha'],
                                       civet_version=civet_version,
                                      no_tests=True)
        self.assertEqual(built_result.results.civet_version, civet_version)
        self.assertIsNone(built_result.results.base_sha)

    def testNoEventID(self):
        """
        Tests when event_id didn't exist in the database (civet_version < 3)
        """
        civet_version = 2
        built_result = self.buildResult(remove_civet_keys=['event_id'],
                                        civet_version=civet_version,
                                        no_tests=True)
        self.assertEqual(built_result.results.civet_version, civet_version)
        self.assertIsNone(built_result.results.event_id)

    def testNoTiming(self):
        """
        Tests timing not being available in a test result
        """
        built_result = self.buildResult(delete_test_key=['timing'])
        for test_result in built_result.results.tests:
            self.assertIsNone(test_result.timing)
            self.assertIsNone(test_result.run_time)
            self.assertIsNone(test_result.hpc_queued_time)

    def testNoRunTime(self):
        """
        Tests run_time not being available in timing in a test result
        """
        built_result = self.buildResult(delete_test_key=['timing', 'runner_run'])
        for test_result in built_result.results.tests:
            self.assertIsNone(test_result.run_time)

    def testNoHPCQueuedTime(self):
        """
        Tests hpc_queued_time not being available in timing in a test result
        """
        built_result = self.buildResult(delete_test_key=['timing', 'hpc_queued'])
        for test_result in built_result.results.tests:
            self.assertIsNone(test_result.hpc_queued_time)

    def testNoTester(self):
        """
        Tests tester not being available in a test result
        """
        built_result = self.buildResult(delete_test_key=['tester'])
        for test_result in built_result.results.tests:
            self.assertIsNone(test_result.tester)
            self.assertIsNone(test_result.json_metadata)

    def testNoStatus(self):
        """
        Tests status not being available in a test result
        """
        built_result = self.buildResult(delete_test_key=['status'])
        for test_result in built_result.results.tests:
            self.assertIsNone(test_result.status)
            self.assertIsNone(test_result.status_value)

    def testNoTestResultID(self):
        """
        Test result_id not being within a test (when tests
        are stored directly within result data)
        """
        built_result = self.buildResult(delete_test_key=['result_id'])
        for test_result in built_result.results.tests:
            self.assertEqual(test_result.result_id, built_result.results.id)

    def _compareSerialized(self, result: StoredResult, new_result: StoredResult,
                           same_tests: bool):
        # Test keys should be the same (values will not be
        # because they were loaded from the database and
        # were originally stored as IDs)
        self.assertEqual(result.data['tests'].keys(), new_result.data['tests'].keys())
        self.assertEqual(result._tests.keys(), new_result._tests.keys())
        # And the underlying test objects should also be the same
        for test_name in result.test_names:
            test = result._tests[test_name]

            new_test = new_result._tests[test_name]
            self.assertEqual(test._data, new_test._data)
            self.assertEqual(test.name, new_test.name)
            self.assertEqual(test._validation_results, new_test._validation_results)
            self.assertEqual(test._validation_data, new_test._validation_data)

        # Compare all of the data
        new_data = deepcopy(new_result.data)
        # If the tests entry isn't exactly the same, just
        # duplicate it
        if not same_tests:
            new_data['tests'] = deepcopy(result.data['tests'])
        self.assertEqual(result.data, new_data)

    @patch.object(StoredResult, '_find_tests_data')
    def testSerializeTestsNotLoaded(self, patch_find_tests_data):
        """
        Test serializing and deserializing a result when the data
        is not already loaded
        """
        captured_result = self.captureResult(test_in_results=False)
        result_data = captured_result.result_data
        test_data = captured_result.test_data

        # Mock getting the test results from mongodb
        def find_tests_data(ids):
            return [v for v in test_data.values() if v['_id'] in ids]
        patch_find_tests_data.side_effect = find_tests_data

        result = StoredResult(result_data)

        # Tests should not be loaded yet
        for entry in result._tests.values():
            self.assertIsInstance(entry, ObjectId)

        serialized = result.serialize()

        # And should be loaded now
        for entry in result._tests.values():
            self.assertIsInstance(entry, StoredTestResult)

        new_result = StoredResult.deserialize_build(serialized)

        # Compare all of the data
        self._compareSerialized(result, new_result, False)

    def testSerializeTestsInResult(self):
        """
        Test serializing and deserializing a result when the test
        data is already loaded in the result data (no database loads)
        """
        built_result = self.buildResult()
        results = built_result.results

        # Tests should be loaded already
        for entry in results._tests.values():
            self.assertIsInstance(entry, StoredTestResult)

        serialized = results.serialize()

        new_result = StoredResult.deserialize_build(serialized)

        # Compare all of the data
        self._compareSerialized(results, new_result, True)

    @patch.object(StoredResult, '_find_tests_data')
    def testLoadAllTestsMissing(self, patch_find_tests_data):
        """
        Tests StoredResult.load_all_data when test(s)
        were not found in the database
        """
        captured_result = self.captureResult(test_in_results=False)

        patch_find_tests_data.return_value = []

        with self.assertRaisesRegex(KeyError, 'Failed to load test results'):
            StoredResult(captured_result.result_data).load_all_tests()


if __name__ == '__main__':
    unittest.main()
