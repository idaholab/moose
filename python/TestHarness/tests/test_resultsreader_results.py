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
from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase
from TestHarness.resultsreader.results import TestHarnessResults, TestHarnessTestResult, TestName, DatabaseException

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
FAKE_JSON_METADATA = {'foo': 'bar'}

class TestResultsReaderResults(TestHarnessTestCase):
    def setUp(self):
        self.run_tests_result = self.runTests('-i', 'validation', '--capture-perf-graph', exit_code=132)

    def captureResult(self, remove_civet_keys=[], civet_version=FAKE_CIVET_VERSION,
                      test_in_results=True, no_tests=False, delete_test_key=None):
        values = self.run_tests_result.results

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

                tester_metadata = test_values.get('tester', {}).get('json_metadata', {})
                for key, value in tester_metadata.items():
                    if value:
                        tester_metadata[key] = zlib.compress(json.dumps(FAKE_JSON_METADATA).encode('utf-8'))

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

        return results, tests

    def buildResult(self, *args, **kwargs) -> Tuple[TestHarnessResults, dict]:
        result_data, test_data = self.captureResult(*args, **kwargs)

        results = TestHarnessResults(result_data)
        if not kwargs.get('no_tests', False):
            self.assertEqual(len(results._tests), len(test_data))

        return results, test_data

    def testTestHarnessResult(self):
        """
        Tests a basic build of a TestHarnessResult and the underlying test data
        """
        result, test_data = self.buildResult()
        harness = self.run_tests_result.harness

        self.assertEqual(result.data, result._data)
        self.assertEqual(result.id, FAKE_RESULT_ID)
        self.assertEqual(result.testharness, result.data['testharness'])
        self.assertEqual(result.version, result.data['testharness']['version'])
        self.assertEqual(result.validation_version, result.data['testharness']['validation_version'])

        # Faked entries for civet
        self.assertEqual(result.civet_job_url, FAKE_CIVET_JOB_URL)
        self.assertEqual(result.civet_job_id, FAKE_CIVET_JOB_ID)
        self.assertEqual(result.civet_version, FAKE_CIVET_VERSION)
        self.assertEqual(result.event_sha, FAKE_EVENT_SHA)
        self.assertEqual(result.event_cause, FAKE_EVENT_CAUSE)
        self.assertEqual(result.event_id, FAKE_EVENT_ID)
        self.assertEqual(result.pr_num, FAKE_PR_NUM)
        self.assertEqual(result.base_sha, FAKE_BASE_SHA)
        self.assertEqual(result.time, FAKE_TIME)

        # Has same number of tests
        for test_result in result.get_tests():
            data = test_data[test_result.name]

            # Get the actual Job object from the TestHarness
            jobs = [j for j in harness.finished_jobs if j.getTestName() == str(test_result.name)]
            self.assertEqual(len(jobs), 1)
            job = jobs[0]

            # Base properties
            self.assertEqual(test_result.data, data)
            self.assertEqual(test_result.id, data['_id'])
            self.assertEqual(test_result.results, result)
            self.assertEqual(test_result.result_id, result.id)
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
            self.assertEqual(test_result.json_metadata, data['tester']['json_metadata'])
            for k, v in data['tester']['json_metadata'].items():
                self.assertEqual(test_result.json_metadata[k], v)

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

    @patch.object(TestHarnessResults, '_find_test_data')
    def testGetTest(self, patch_find_test_data):
        """
        Tests when tests are stored as ObjectId (reference to another collection)
        and then later loaded from the database on request
        """
        result_data, test_data = self.captureResult(test_in_results=False)

        # Mock getting the test results from mongodb
        def get_test_data(id):
            for test in test_data.values():
                if test['_id'] == id:
                    return test
            return None
        patch_find_test_data.side_effect = get_test_data

        result = TestHarnessResults(result_data)
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
        result, _ = self.buildResult()

        self.assertFalse(result.has_test('foo', 'bar'))
        with self.assertRaisesRegex(KeyError, 'Test "foo.bar" does not exist'):
            result.get_test('foo', 'bar')

    @patch.object(TestHarnessResults, '_find_test_data')
    def testGetTestMissingDatabase(self, patch_find_test_data):
        """
        Tests TestHarnessResult.get_test when a test does not exist
        in the database
        """
        patch_find_test_data.side_effect = lambda id: None

        result, _ = self.buildResult(test_in_results=False)

        for name in result.test_names:
            id = result._tests[name]
            with self.assertRaisesRegex(DatabaseException, f'Database missing tests._id={id}'):
                result.get_test(name.folder, name.name)

    @patch.object(TestHarnessTestResult, '__init__')
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
        result, _ = self.captureResult()

        for folder_name, folder_entry in result['tests'].items():
            for test_name in list(folder_entry['tests'].keys()):
                folder_entry['tests'][test_name] = 1

        with self.assertRaisesRegex(TypeError, 'has unexpected type "int"'):
            TestHarnessResults(result)

    def testDeprecatedBaseSHA(self):
        """
        Tests when base_sha didn't exist in the database (civet version < 2)
        """
        civet_version = 1
        result, _, = self.buildResult(remove_civet_keys=['base_sha'],
                                      civet_version=civet_version,
                                      no_tests=True)
        self.assertEqual(result.civet_version, civet_version)
        self.assertIsNone(result.base_sha)

    def testNoEventID(self):
        """
        Tests when event_id didn't exist in the database (civet_version < 3)
        """
        civet_version = 2
        result, _ = self.buildResult(remove_civet_keys=['event_id'],
                                       civet_version=civet_version,
                                       no_tests=True)
        self.assertEqual(result.civet_version, civet_version)
        self.assertIsNone(result.event_id)

    def testNoTiming(self):
        """
        Tests timing not being available in a test result
        """
        result, _ = self.buildResult(delete_test_key=['timing'])
        for test_result in result.get_tests():
            self.assertIsNone(test_result.timing)
            self.assertIsNone(test_result.run_time)
            self.assertIsNone(test_result.hpc_queued_time)

    def testNoRunTime(self):
        """
        Tests run_time not being available in timing in a test result
        """
        result, _ = self.buildResult(delete_test_key=['timing', 'runner_run'])
        for test_result in result.get_tests():
            self.assertIsNone(test_result.run_time)

    def testNoHPCQueuedTime(self):
        """
        Tests hpc_queued_time not being available in timing in a test result
        """
        result, _ = self.buildResult(delete_test_key=['timing', 'hpc_queued'])
        for test_result in result.get_tests():
            self.assertIsNone(test_result.hpc_queued_time)

    def testNoTester(self):
        """
        Tests tester not being available in a test result
        """
        result, _ = self.buildResult(delete_test_key=['tester'])
        for test_result in result.get_tests():
            self.assertIsNone(test_result.tester)
            self.assertIsNone(test_result.json_metadata)

    def testNoStatus(self):
        """
        Tests status not being available in a test result
        """
        result, _ = self.buildResult(delete_test_key=['status'])
        for test_result in result.get_tests():
            self.assertIsNone(test_result.status)
            self.assertIsNone(test_result.status_value)

    def _compareSerialized(self, result: TestHarnessResults, new_result: TestHarnessResults,
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

    @patch.object(TestHarnessResults, '_find_test_data')
    def testSerializeTestsNotLoaded(self, patch_find_test_data):
        """
        Test serializing and deserializing a result when the data
        is not already loaded
        """
        result_data, test_data = self.captureResult(test_in_results=False)

        # Mock getting the test results from mongodb
        def get_test_data(id):
            for test in test_data.values():
                if test['_id'] == id:
                    return test
            return None
        patch_find_test_data.side_effect = get_test_data

        result = TestHarnessResults(result_data)

        # Tests should not be loaded yet
        for entry in result._tests.values():
            self.assertIsInstance(entry, ObjectId)

        serialized = result.serialize()

        # And should be loaded now
        for entry in result._tests.values():
            self.assertIsInstance(entry, TestHarnessTestResult)

        new_result = TestHarnessResults.deserialize_build(serialized)

        # Compare all of the data
        self._compareSerialized(result, new_result, False)

    def testSerializeTestsInResult(self):
        """
        Test serializing and deserializing a result when the test
        data is already loaded in the result data (no database loads)
        """
        result, _ = self.buildResult()

        # Tests should be loaded already
        for entry in result._tests.values():
            self.assertIsInstance(entry, TestHarnessTestResult)

        serialized = result.serialize()

        new_result = TestHarnessResults.deserialize_build(serialized)

        # Compare all of the data
        self._compareSerialized(result, new_result, True)

if __name__ == '__main__':
    unittest.main()
