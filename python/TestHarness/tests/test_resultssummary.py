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
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        results, head_names, base_names = summary.pr_test_names(event_id=EVENT_ID)
        removed_table, added_table = summary.diff_table(results, base_names, head_names)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableRemovedTest(self):
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        results, _, base_names = summary.pr_test_names(event_id=EVENT_ID)
        removed_table, added_table = summary.diff_table(results, base_names, set())
        self.assertEqual(len(removed_table), 1)
        self.assertEqual(removed_table[0], TEST_NAME)
        self.assertIsNone(added_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableAddedTest(self):
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        results, head_names , _ = summary.pr_test_names(event_id=EVENT_ID)
        removed_table, added_table = summary.diff_table(results, set(), head_names)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id=EVENT_ID)
        test_result = results.get_test(TEST_NAME.folder, TEST_NAME.name)
        
        self.assertEqual(len(added_table), 1)
        self.assertEqual(added_table[0][0], str(TEST_NAME))
        self.assertEqual(added_table[0][1], test_result.run_time)
        self.assertIsNone(removed_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoChanges(self):
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        _, head_names, base_names = summary.pr_test_names(event_id=EVENT_ID)
        self.assertEqual(head_names, base_names)
        self.assertEqual(head_names, set([TEST_NAME]))
        self.assertEqual(base_names, set([TEST_NAME]))

    @patch.object(TestHarnessResultsReader, 'getCommitResults')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoBaseResults(self, patch_commit_results):
        patch_commit_results.return_value = None
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        stdout = StringIO()
        with redirect_stdout(stdout):
            results, test_names, base_test_names = summary.pr_test_names(event_id=EVENT_ID)
        self.assertIsNone(base_test_names)

    @patch.object(TestHarnessResultsReader, 'getEventResults')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoEventResults(self, patch_event_results):
        patch_event_results.return_value = None
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        with self.assertRaisesRegex(SystemExit, 'Results do not exist for event'):
            summary.pr_test_names(event_id=EVENT_ID)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testBuildSummary(self):
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        results, head_names , _ = summary.pr_test_names(event_id=EVENT_ID)
        removed_table, added_table = summary.diff_table(results, set(), head_names)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id=EVENT_ID)
        test_result = results.get_test(TEST_NAME.folder, TEST_NAME.name)
        
        self.assertEqual(len(added_table), 1)
        self.assertEqual(added_table[0][0], str(TEST_NAME))
        self.assertEqual(added_table[0][1], test_result.run_time)
        self.assertIsNone(removed_table)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPR(self):
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        
        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.pr(event_id=EVENT_ID)
        self.assertIn('Removed Tests:', stdout.getvalue())
        self.assertIn('No Removed Tests', stdout.getvalue())
        self.assertIn('New Tests:', stdout.getvalue())
        self.assertIn('No Removed Tests', stdout.getvalue())

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRNoBase(self):
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)
        def mock_pr_test_names(**kwargs):
            return [], ['head_test1'], []
        summary.pr_test_names = mock_pr_test_names

        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.pr(event_id=EVENT_ID)
        self.assertIn('Skipping', stdout.getvalue())

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testMain(self):
        summary = TestHarnessResultsSummary(TEST_DATABASE_NAME)

        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.main(action='pr', event_id=EVENT_ID)
        self.assertIn('Removed Tests:', stdout.getvalue())
        self.assertIn('No Removed Tests', stdout.getvalue())
        self.assertIn('New Tests:', stdout.getvalue())
        self.assertIn('No Removed Tests', stdout.getvalue())

if __name__ == '__main__':
    unittest.main()
