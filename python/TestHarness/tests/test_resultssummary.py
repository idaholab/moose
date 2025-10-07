#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import os
import tempfile
from mock import patch
from contextlib import redirect_stdout
from io import StringIO

from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultssummary.summary import TestHarnessResultsSummary
from TestHarness.resultsstore.storedresults import TestName, StoredResult

from TestHarnessTestCase import TestHarnessTestCase
from test_resultsstore_storedresults import TestResultsStoredResults

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = ResultsReader.hasEnvironmentAuthentication()

# Test database name for testing pull request results
TEST_DATABASE_NAME = 'civet_tests_moose_test_results'
# Arguments for using the production database for getting real results
TEST_FOLDER_NAME = 'tests/test_harness'
TEST_TEST_NAME = 'ok'
TEST_NAME = TestName(TEST_FOLDER_NAME, TEST_TEST_NAME)

MOCKED_TEST_NAME = TestName('tests/test_harness', 'always_ok')

EVENT_ID = os.environ.get('CIVET_EVENT_ID')
if EVENT_ID is not None:
    EVENT_ID = int(EVENT_ID)

class TestResultsSummary(TestHarnessTestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def getResult(self, **kwargs) -> StoredResult:
        run_test_result = self.runTestsCached('-i', 'always_ok')
        converted = TestResultsStoredResults.convertTestHarnessResults(run_test_result.results, **kwargs)
        return StoredResult(converted.result_data)

    def testFoo(self):
        # Change runtime
        head_result = self.getResult()
        base_result = self.getResult()

        head_test = head_result.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        base_test = base_result.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        print(head_test.run_time)
        print(base_test.run_time)
        base_test._data['timing']['runner_run'] = 1e6
        print(base_test.run_time)

        # Add a test
        head_result = self.getResult(no_tests=True)
        base_result = self.getResult()

        print(head_result.num_tests)
        print(base_result.num_tests)

    #@patch.object(TestHarnessResultsSummary, 'HEAD_RUNTIME_THREADSHOLD',new=1)
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableNoChanges(self):
        """
        Tests diff_table when there are no changes between base and head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        base_results, head_results, base_names, head_names = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        removed_table, added_table, same_table = summary.diff_table(base_results, head_results, base_names, head_names)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testFooBar(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        head_result_no_tests = self.getResult(no_tests=True)
        base_result_with_tests = self.getResult()
        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_no_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as tmp_file:
            summary = TestHarnessResultsSummary(None)
            base_results, head_results, base_test_names, head_test_names = summary.pr_test_names(
                event_id=EVENT_ID, out=tmp_file.name
            )
            self.assertEqual(base_results, base_result_with_tests)
            self.assertEqual(head_results, head_result_no_tests)
            self.assertEqual(len(head_test_names), 0)
            self.assertEqual(len(base_test_names), base_results.num_tests)

        TestHarnessResultsSummary(None)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableRemovedTest(self):
        """
        Tests diff_table() when test is removed from base.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        base_results, head_results,base_names, _= summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        removed_table, added_table, same_table = summary.diff_table(base_results, head_results, base_names, set())
        self.assertEqual(len(removed_table), 1)
        self.assertEqual(removed_table[0], TEST_NAME)
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableAddedTest(self):
        """
        Tests diff_table() when test is newly added.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        base_results, head_results, _, head_names = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        removed_table, added_table, same_table = summary.diff_table(base_results, head_results, set(), head_names)

        reader = ResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id=EVENT_ID)
        test_result = results.get_test(TEST_NAME.folder, TEST_NAME.name)

        self.assertEqual(len(added_table), 1)
        self.assertEqual(added_table[0][0], str(TEST_NAME))
        self.assertEqual(added_table[0][1], test_result.run_time)
        self.assertIsNone(removed_table)
        self.assertIsNone(same_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableSameTestHighRelativeRunTime(self):
        """
        Tests diff_table() when same test name exit in both base and head, where:
            -the head runtime exceeds a predefined threshold and
            -the relative runtime increase exceeds a defined rate.
        """
        fake_run_time_floor = 0.1
        fake_run_tiime_rate_floor = 0.1
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        base_results, head_results, base_names, head_names = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        removed_table, added_table, same_table = summary.diff_table(
                                                                    base_results,
                                                                    head_results,
                                                                    base_names,
                                                                    head_names,
                                                                    run_time_floor=fake_run_time_floor,
                                                                    run_time_rate_floor=fake_run_tiime_rate_floor)

        reader = ResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id=EVENT_ID)
        test_result = results.get_test(TEST_NAME.folder, TEST_NAME.name)

        base_results = reader.getCommitResults(results.base_sha)
        base_result = base_results.get_test(TEST_NAME.folder, TEST_NAME.name)

        self.assertEqual(len(same_table), 1)
        self.assertEqual(same_table[0][0], str(TEST_NAME))
        self.assertEqual(same_table[0][1], base_result.run_time)
        self.assertEqual(same_table[0][2], test_result.run_time)

        relative_diff_str = same_table[0][3]
        relative_diff = float(relative_diff_str.strip('%')) / 100
        self.assertGreater(relative_diff, fake_run_time_floor)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoChanges(self):
        """
        Tests pr_test_names() when there are no changes between base and head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        _, _, base_names, head_names = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        self.assertEqual(base_names, head_names)
        self.assertEqual(base_names, set([TEST_NAME]))
        self.assertEqual(head_names, set([TEST_NAME]))

    @patch.object(ResultsReader, 'getCommitResults')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoBaseResults(self, patch_commit_results):
        """
        Tests pr_test_names() when no base is available to compare.
        """
        patch_commit_results.return_value = None
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        stdout = StringIO()
        with redirect_stdout(stdout):
            base_results, head_results, base_test_names, test_names = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        self.assertIsNone(base_test_names)

    @patch.object(ResultsReader, 'getEventResults')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoEventResults(self, patch_event_results):
        """
        Tests pr_test_names() when no event results are available.
        """
        patch_event_results.return_value = None
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        with self.assertRaisesRegex(SystemExit, 'Results do not exist for event'):
            summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testBuildSummaryNoChange(self):
        """
        Tests building a summary when there are no changes between base and head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        base_results, head_results, base_names, head_names = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        removed_table, added_table, same_table = summary.diff_table(base_results, head_results,base_names,head_names)

        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testBuildSummaryRemovedTest(self):
        """
        Tests building a summary when a test is removed from base
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        base_results, head_results, base_names, _ = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        removed_table, added_table, same_table = summary.diff_table(base_results,head_results,base_names,set())

        self.assertEqual(len(removed_table), 1)
        self.assertEqual(removed_table[0], TEST_NAME)
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testBuildSummaryAddedTest(self):
        """
        Tests building a summary when a test is newly added in the head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        base_results, head_results, _, head_names = summary.pr_test_names(event_id=EVENT_ID,out=tmp_path)
        removed_table, added_table,same_table = summary.diff_table(base_results, head_results, set(), head_names)

        reader = ResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id=EVENT_ID)
        test_result = results.get_test(TEST_NAME.folder, TEST_NAME.name)

        self.assertEqual(len(added_table), 1)
        self.assertEqual(added_table[0][0], str(TEST_NAME))
        self.assertEqual(added_table[0][1], test_result.run_time)
        self.assertIsNone(removed_table)
        self.assertIsNone(same_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRNoChange(self):
        """
        Tests pr() when there are no changes between base and head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        stdout = StringIO()
        with redirect_stdout(stdout):
            summary_result = summary.pr(event_id=EVENT_ID,out=tmp_path)
        self.assertIn('Removed Tests:',summary_result)
        self.assertIn('No Removed Tests',summary_result)
        self.assertIn('New Tests:',summary_result)
        self.assertIn('No New Tests',summary_result)
        self.assertIn('Same Tests',summary_result)
        self.assertIn('No Tests',summary_result)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRNoBase(self):
        """
        Tests pr() when no base is available to compare.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        def mock_pr_test_names(**kwargs):
            print("Comparison not available")
            return None,[], None,{'head_test1'}
        summary.pr_test_names = mock_pr_test_names

        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.pr(event_id=EVENT_ID,out=tmp_path)
        self.assertIn('Comparison not available', stdout.getvalue())

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testSummaryOutputFileValidPath(self):
        """
        Tests summary output file when output file path exit and there is no change between base and head.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
        stdout = StringIO()
        with redirect_stdout(stdout):
            summary_result = summary.pr(event_id=EVENT_ID, out=tmp_path)

            summary.summary_output_file(summary_result,tmp_path)
            with open(tmp_path, 'r') as f:
                output = f.read()
            self.assertIn('Removed Tests:', output)
            self.assertIn('No Removed Tests', output)
            self.assertIn('New Tests:', output)
            self.assertIn('No New Tests', output)
            self.assertIn('Same Tests', output)
            self.assertIn('No Tests', output)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testSummaryOutputFileInvalidPath(self):
        """
        Tests that summary_output_file when output file path is invalid.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        invalid_path = "/this/path/does/not/exist/output.txt"
        stdout = StringIO()
        with redirect_stdout(stdout):
            summary_result = summary.pr(event_id=EVENT_ID, out=invalid_path)

        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.summary_output_file(summary_result, invalid_path)

        output = stdout.getvalue()
        self.assertIn("Failed to write to", output)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testMain(self):
        """
        Tests main() output for a PR event, verifying summary messages are printed in output file path.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)

        with tempfile.NamedTemporaryFile() as tmp_file:
            tmp_path = tmp_file.name
            stdout = StringIO()
            with redirect_stdout(stdout):
                summary.main( out=tmp_path, action='pr', event_id=EVENT_ID)
            with open(tmp_path, 'r') as f:
                output = f.read()
            self.assertIn('Removed Tests:', output)
            self.assertIn('No Removed Tests', output)
            self.assertIn('New Tests:', output)
            self.assertIn('No New Tests', output)
            self.assertIn('Same Tests', output)
            self.assertIn('No Tests', output)

if __name__ == '__main__':
    unittest.main()
