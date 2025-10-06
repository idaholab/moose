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

from TestHarness.resultsreader.reader import TestHarnessResultsReader
from TestHarness.resultssummary.summary import TestHarnessResultsSummary
from TestHarness.resultsreader.results import TestName

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = TestHarnessResultsReader.hasEnvironmentAuthentication()

# Test database name for testing pull request results
TEST_DATABASE_NAME = 'civet_tests_moose_test_results'
# Arguments for using the production database for getting real results
TEST_FOLDER_NAME = 'tests/test_harness'
TEST_TEST_NAME = 'ok'
TEST_NAME = TestName(TEST_FOLDER_NAME, TEST_TEST_NAME)

EVENT_ID = os.environ.get('CIVET_EVENT_ID')
if EVENT_ID is not None:
    EVENT_ID = int(EVENT_ID)

class TestResultsSummary(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableNoChanges(self):
        """
        Tests diff_table when there are no changes between base and head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        base_results, head_results, base_names, head_names = summary.pr_test_names(event_id=EVENT_ID)
        removed_table, added_table, same_table = summary.diff_table(base_results, head_results, base_names, head_names)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableRemovedTest(self):
        """
        Tests diff_table() when test is removed from base.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        base_results, head_results,base_names, _= summary.pr_test_names(event_id=EVENT_ID)
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
        base_results, head_results, _, head_names = summary.pr_test_names(event_id=EVENT_ID)
        removed_table, added_table, same_table = summary.diff_table(base_results, head_results, set(), head_names)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id=EVENT_ID)
        test_result = results.get_test(TEST_NAME.folder, TEST_NAME.name)

        self.assertEqual(len(added_table), 1)
        self.assertEqual(added_table[0][0], str(TEST_NAME))
        self.assertEqual(added_table[0][1], test_result.run_time)
        self.assertIsNone(removed_table)
        self.assertIsNone(same_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoChanges(self):
        """
        Tests pr_test_names() when there are no changes between base and head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        _, _, base_names, head_names = summary.pr_test_names(event_id=EVENT_ID)
        self.assertEqual(base_names, head_names)
        self.assertEqual(base_names, set([TEST_NAME]))
        self.assertEqual(head_names, set([TEST_NAME]))

    @patch.object(TestHarnessResultsReader, 'getCommitResults')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoBaseResults(self, patch_commit_results):
        """
        Tests pr_test_names() when no base is available to compare.
        """
        patch_commit_results.return_value = None
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        stdout = StringIO()
        with redirect_stdout(stdout):
            base_results, head_results, base_test_names, test_names = summary.pr_test_names(event_id=EVENT_ID)
        self.assertIsNone(base_test_names)

    @patch.object(TestHarnessResultsReader, 'getEventResults')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoEventResults(self, patch_event_results):
        """
        Tests pr_test_names() when no event results are available.
        """
        patch_event_results.return_value = None
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with self.assertRaisesRegex(SystemExit, 'Results do not exist for event'):
            summary.pr_test_names(event_id=EVENT_ID)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testBuildSummaryNoChange(self):
        """
        Tests building a summary when there are no changes between base and head test names.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        base_results, head_results, base_names, head_names = summary.pr_test_names(event_id=EVENT_ID)
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
        base_results, head_results, base_names, _ = summary.pr_test_names(event_id=EVENT_ID)
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
        base_results, head_results, _, head_names = summary.pr_test_names(event_id=EVENT_ID)
        removed_table, added_table,same_table = summary.diff_table(base_results, head_results, set(), head_names)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
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

        summary_result = summary.pr(event_id=EVENT_ID)
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
        def mock_pr_test_names(**kwargs):
            print("Comparison not available")
            return None,[], None,{'head_test1'}
        summary.pr_test_names = mock_pr_test_names

        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.pr(event_id=EVENT_ID)
        self.assertIn('Comparison not available', stdout.getvalue())

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testSummaryOutputFileValidPath(self):
        """
        Tests summary output file when output file path exit and there is no change between base and head.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        summary_result = summary.pr(event_id=EVENT_ID)

        with tempfile.NamedTemporaryFile(delete=False, mode='r+') as tmp_file:
            tmp_path = tmp_file.name
        try:
            summary.summary_output_file(summary_result,tmp_path)
            with open(tmp_path, 'r') as f:
                output = f.read()
            self.assertIn('Removed Tests:', output)
            self.assertIn('No Removed Tests', output)
            self.assertIn('New Tests:', output)
            self.assertIn('No New Tests', output)
            self.assertIn('Same Tests', output)
            self.assertIn('No Tests', output)
        finally:
            os.remove(tmp_path)


    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testSummaryOutputFileInvalidPath(self):
        """
        Tests that summary_output_file when output file path is invalid.
        """
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        summary_result = summary.pr(event_id=EVENT_ID)

        invalid_path = "/this/path/does/not/exist/output.txt"

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

        with tempfile.NamedTemporaryFile(delete=False, mode='r+') as tmp_file:
            tmp_path = tmp_file.name
        try:
            summary.main( out=tmp_path, action='pr', event_id=EVENT_ID)
            with open(tmp_path, 'r') as f:
                output = f.read()
            self.assertIn('Removed Tests:', output)
            self.assertIn('No Removed Tests', output)
            self.assertIn('New Tests:', output)
            self.assertIn('No New Tests', output)
            self.assertIn('Same Tests', output)
            self.assertIn('No Tests', output)
        finally:
            os.remove(tmp_path)

if __name__ == '__main__':
    unittest.main()
