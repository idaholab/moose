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
from datetime import datetime
from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase
from TestHarness.resultsreader.results import TestHarnessResults, TestHarnessTestResult

FAKE_CIVET_VERSION = 2
FAKE_CIVET_JOB_ID = 12345
FAKE_CIVET_JOB_URL = f'civet.inl.gov/job/{FAKE_CIVET_JOB_ID}'
FAKE_EVENT_SHA = 'abcd1234ababcd1234ababcd1234ababcd1234ab'
FAKE_BASE_SHA = '1234abcdab1234abcdab1234abcdab1234abcdab'
FAKE_EVENT_CAUSE = 'pr'
FAKE_PR_NUM = 1234
FAKE_HPC_QUEUED_TIME = 1.234
FAKE_TIME = datetime.now()

class TestResultsReaderResults(TestHarnessTestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.run_tests_result = self.runTests('-i', 'validation', '--capture-perf-graph', exit_code=132)

    def captureResult(self, remove_civet_keys=[], civet_version=FAKE_CIVET_VERSION):
        values = self.run_tests_result.results.copy()

        tests = []

        # Faked values that CIVET would add
        civet_values = {'event_sha': FAKE_EVENT_SHA,
                        'event_cause': FAKE_EVENT_CAUSE,
                        'pr_num': FAKE_PR_NUM,
                        'base_sha': FAKE_BASE_SHA,
                        'time': FAKE_TIME}
        for k in remove_civet_keys:
            del civet_values[k]

        # Perform fixups that CIVET would do
        for folder_name, folder_values in values['tests'].items():
            for test_name, test_values in folder_values['tests'].items():
                # Remove output entires, as they're removed when storing
                for key in ['output', 'output_files']:
                    if key in test_values:
                        del test_values[key]

                tester_metadata = test_values.get('tester', {}).get('json_metadata', {})
                for key, value in tester_metadata.items():
                    if value:
                        with open(value, 'r') as f:
                            data = json.load(f)
                        tester_metadata[key] = zlib.compress(json.dumps(data).encode('utf-8'))

                test_values['folder_name'] = folder_name
                test_values['test_name'] = test_name

                # Fake values from civet
                test_values.update(civet_values)
                # Fake values from HPC
                test_values['timing']['hpc_queued'] = FAKE_HPC_QUEUED_TIME
                tests.append(test_values)

        # Setup main results entry
        results = values.copy()
        del results['tests']
        results['civet'] = {'job_url': FAKE_CIVET_JOB_URL,
                            'job_id': FAKE_CIVET_JOB_ID,
                            'version': civet_version}
        results.update(civet_values)

        return results, tests

    def testTestHarnessResult(self):
        results, _ = self.captureResult()
        result = TestHarnessResults(results)

        self.assertEqual(result.data, results)
        self.assertEqual(result.testharness, results['testharness'])
        self.assertEqual(result.version, results['testharness']['version'])
        self.assertEqual(result.validation_version, results['testharness']['validation_version'])

        # Faked entries for civet
        self.assertEqual(result.civet_job_url, FAKE_CIVET_JOB_URL)
        self.assertEqual(result.civet_job_id, FAKE_CIVET_JOB_ID)
        self.assertEqual(result.civet_version, FAKE_CIVET_VERSION)
        self.assertEqual(result.event_sha, FAKE_EVENT_SHA)
        self.assertEqual(result.event_cause, FAKE_EVENT_CAUSE)
        self.assertEqual(result.pr_num, FAKE_PR_NUM)
        self.assertEqual(result.base_sha, FAKE_BASE_SHA)
        self.assertEqual(result.time, FAKE_TIME)

    def testTestResult(self):
        results, tests = self.captureResult()
        test_harness_results = TestHarnessResults(results)
        harness = self.run_tests_result.harness

        for entry in tests:
            # Get the actual Job object from the TestHarness
            folder_name = entry["folder_name"]
            test_name = entry["test_name"]
            full_test_name = f'{folder_name}.{test_name}'
            jobs = [j for j in harness.finished_jobs if j.getTestName() == full_test_name]
            self.assertEqual(len(jobs), 1)
            job = jobs[0]

            test_result = TestHarnessTestResult(entry, test_harness_results)

            # Base properties
            self.assertEqual(test_result.data, entry)
            self.assertEqual(test_result.results, test_harness_results)
            self.assertEqual(test_result.folder_name, folder_name)
            self.assertEqual(test_result.test_name, test_name)

            # Faked CIVET properties
            self.assertEqual(test_result.event_sha, FAKE_EVENT_SHA)
            self.assertEqual(test_result.event_cause, FAKE_EVENT_CAUSE)
            self.assertEqual(test_result.pr_num, FAKE_PR_NUM)
            self.assertEqual(test_result.base_sha, FAKE_BASE_SHA)
            self.assertEqual(test_result.time, FAKE_TIME)

            # Status properties
            self.assertEqual(test_result.status, entry['status'])
            self.assertEqual(test_result.status_value, entry['status']['status'])

            # Timing properties
            self.assertEqual(test_result.timing, entry['timing'])
            self.assertEqual(test_result.run_time, entry['timing']['runner_run'])
            self.assertEqual(test_result.hpc_queued_time, FAKE_HPC_QUEUED_TIME)

            # Tester properties
            self.assertEqual(test_result.tester, entry['tester'])
            self.assertEqual(test_result.json_metadata, entry['tester']['json_metadata'])
            for k, v in entry['tester']['json_metadata'].items():
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

    def testDeprecatedBaseSHA(self):
        """
        Tests when base_sha didn't exist in the database (civet version < 2)
        """
        civet_version = 1
        results, tests = self.captureResult(remove_civet_keys=['base_sha'],
                                            civet_version=civet_version)

        test_harness_results = TestHarnessResults(results)
        self.assertEqual(test_harness_results.civet_version, civet_version)
        self.assertIsNone(test_harness_results.base_sha)

        for entry in tests:
            test_result = TestHarnessTestResult(entry, test_harness_results)
            self.assertIsNone(test_result.base_sha)

if __name__ == '__main__':
    unittest.main()
