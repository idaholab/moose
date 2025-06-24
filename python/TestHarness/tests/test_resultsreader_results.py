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

FAKE_CIVET_VERSION = 1
FAKE_CIVET_JOB_URL = 'civet.inl.gov/job/12345'
FAKE_EVENT_SHA = 'abcd1234ababcd1234ababcd1234ababcd1234ab'
FAKE_EVENT_CAUSE = 'pr'
FAKE_PR_NUM = 1234
FAKE_HPC_QUEUED_TIME = 1.234
FAKE_TIME = datetime.now()

class TestResultsReaderResults(TestHarnessTestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.results, self.tests, self.harness = self.captureResult()

    def captureResult(self):
        args = ['-i', 'validation', '--capture-perf-graph']

        run_tests_result = self.runTests(*args, exit_code=132)
        values = run_tests_result.results
        harness = run_tests_result.harness

        tests = []

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
                test_values['event_sha'] = FAKE_EVENT_SHA
                test_values['event_cause'] = FAKE_EVENT_CAUSE
                test_values['pr_num'] = FAKE_PR_NUM
                test_values['time'] = FAKE_TIME
                # Fake values from HPC
                test_values['timing']['hpc_queued'] = FAKE_HPC_QUEUED_TIME
                tests.append(test_values)

        # Setup main results entry
        results = values.copy()
        del results['tests']
        results['civet'] = {'job_url': FAKE_CIVET_JOB_URL,
                            'version': FAKE_CIVET_VERSION}
        results['event_sha'] = FAKE_EVENT_SHA
        results['event_cause'] = FAKE_EVENT_CAUSE
        results['pr_num'] = FAKE_PR_NUM
        results['time'] = FAKE_TIME

        return results, tests, harness

    def testTestHarnessResult(self):
        results = self.results.copy()
        result = TestHarnessResults(self.results.copy())

        self.assertEqual(result.data, results)
        self.assertEqual(result.testharness, results['testharness'])
        self.assertEqual(result.version, results['testharness']['version'])
        self.assertEqual(result.validation_version, results['testharness']['validation_version'])

        # Faked entries for civet
        self.assertEqual(result.civet_job_url, FAKE_CIVET_JOB_URL)
        self.assertEqual(result.civet_version, FAKE_CIVET_VERSION)
        self.assertEqual(result.event_sha, FAKE_EVENT_SHA)
        self.assertEqual(result.event_cause, FAKE_EVENT_CAUSE)
        self.assertEqual(result.pr_num, FAKE_PR_NUM)
        self.assertEqual(result.time, FAKE_TIME)

    def testTestResult(self):
        test_harness_results = TestHarnessResults(self.results.copy())

        tests = self.tests.copy()

        for entry in tests:
            # Get the actual Job object from the TestHarness
            folder_name = entry["folder_name"]
            test_name = entry["test_name"]
            full_test_name = f'{folder_name}.{test_name}'
            jobs = [j for j in self.harness.finished_jobs if j.getTestName() == full_test_name]
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

if __name__ == '__main__':
    unittest.main()
