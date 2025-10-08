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
from tabulate import tabulate

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

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoChanges(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests pr_test_names() when there are no changes between base and head test names.
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as tmp_file:
            summary = TestHarnessResultsSummary(None)
            base_results, head_results, base_test_names, head_test_names = summary.pr_test_names(
                event_id=EVENT_ID, out=tmp_file.name
            )
            self.assertEqual(base_results, base_result_with_tests)
            self.assertEqual(head_results, head_result_with_tests)
            self.assertEqual(len(head_test_names), head_results.num_tests)
            self.assertEqual(len(base_test_names), base_results.num_tests)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoBaseResults(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests pr_test_names() when no base is available to compare.
        """
        head_result_with_tests = self.getResult()
        #set no base
        mock_get_commit_results.return_value = None
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as tmp_file:
            summary = TestHarnessResultsSummary(None)
            stdout = StringIO()
            with redirect_stdout(stdout):
                base_results, head_results, base_test_names, head_test_names = summary.pr_test_names(
                event_id=EVENT_ID, out=tmp_file.name
                )
                self.assertIsNone(base_test_names)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testPRTestNamesNoEventResults(self, mock_get_event_results, mock_init_reader):
        """
        Tests pr_test_names() when no event results are available.
        """
        mock_get_event_results.return_value = None

        with tempfile.NamedTemporaryFile() as tmp_file:
            summary = TestHarnessResultsSummary(None)
            with self.assertRaisesRegex(SystemExit, 'Results do not exist for event'):
                summary.pr_test_names(event_id=EVENT_ID,out=tmp_file.name)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableNoChanges(self,mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests  diff_table() when there are no changes between base and head test names.
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(base_result_with_tests,
                                                                    head_result_with_tests,
                                                                    set(base_result_with_tests.test_names),
                                                                    set(head_result_with_tests.test_names))
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableRemovedTest(self,mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when test is removed from base.
        """
        base_result_with_tests = self.getResult()
        head_result_no_tests = self.getResult(no_tests=True)
        base_test_names = set(base_result_with_tests.test_names)
        head_test_no_names = set(head_result_no_tests.test_names)

        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_no_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(base_result_with_tests,
                                                                    head_result_no_tests,
                                                                    base_test_names,
                                                                    head_test_no_names)

        self.assertEqual(len(removed_table),base_result_with_tests.num_tests)
        self.assertEqual(removed_table[0],str(MOCKED_TEST_NAME))
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableAddedTest(self,mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when test is newly added.
        """
        base_result_no_tests = self.getResult(no_tests=True)
        head_result_with_tests = self.getResult()
        base_test_no_names = set(base_result_no_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = base_result_no_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)

        removed_table, added_table, same_table = summary.diff_table(base_result_no_tests,
                                                                    head_result_with_tests,
                                                                    base_test_no_names,
                                                                    head_test_names)

        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        self.assertEqual(len(added_table), head_result_with_tests.num_tests)
        self.assertEqual(added_table[0][0], str(MOCKED_TEST_NAME))
        self.assertEqual(added_table[0][1], head_test.run_time)
        self.assertIsNone(removed_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableSameTestLowHeadRunTime(self,mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when same test name exit in both base and head but head runtime is lower than threadshold
        It will skip same test name runtime comparison
        """
        fake_run_time_floor = 1

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        base_test_names = set(head_result_with_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)

        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test._data['timing']['runner_run'] = 0.5

        removed_table, added_table, same_table = summary.diff_table(
                                                                    base_result_with_tests,
                                                                    head_result_with_tests,
                                                                    base_test_names,
                                                                    head_test_names,
                                                                    run_time_floor=fake_run_time_floor)

        self.assertIsNone(same_table)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableSameTestLowRelativeRunTime(self,mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when same test name exit in both base and head and relative run time rate is lower than threadshold.
        That testname will not be included in same_table
        """
        fake_run_time_floor = 1
        fake_run_time_rate_floor = 0.5

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        base_test_names = set(head_result_with_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)

        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        base_test._data['timing']['runner_run'] = 10
        head_test._data['timing']['runner_run'] = 13

        removed_table, added_table, same_table = summary.diff_table(
                                                                    base_result_with_tests,
                                                                    head_result_with_tests,
                                                                    base_test_names,
                                                                    head_test_names,
                                                                    run_time_floor=fake_run_time_floor,
                                                                    run_time_rate_floor=fake_run_time_rate_floor)

        self.assertIsNone(same_table)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testDiffTableSameTestHighRelativeRunTime(self,mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when same test name exit in both base and head, where:
            -the head runtime exceeds a predefined threshold and
            -the relative runtime increase exceeds a defined rate.
        """
        fake_run_time_floor = 1
        fake_run_time_rate_floor = 0.5

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        base_test_names = set(head_result_with_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)

        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        base_test._data['timing']['runner_run'] = 10
        head_test._data['timing']['runner_run'] = 17

        removed_table, added_table, same_table = summary.diff_table(
                                                                    base_result_with_tests,
                                                                    head_result_with_tests,
                                                                    base_test_names,
                                                                    head_test_names,
                                                                    run_time_floor=fake_run_time_floor,
                                                                    run_time_rate_floor=fake_run_time_rate_floor)

        self.assertEqual(len(same_table), 1)
        self.assertEqual(same_table[0][0], str(MOCKED_TEST_NAME))
        self.assertEqual(same_table[0][1], base_test.run_time)
        self.assertEqual(same_table[0][2], head_test.run_time)
        self.assertGreater(same_table[0][2], fake_run_time_floor)
        #compare relative run time is higher than floor rate
        self.assertGreater(float(same_table[0][3].strip('%')), fake_run_time_rate_floor * 100)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testFormatRemovedTableNoRemovedTest(self,mock_init_reader):
        """
        Tests formatting remove table when there is no removed tests.
        """
        mock_init_reader.return_value = None
        removed_table = None
        summary = TestHarnessResultsSummary(None)
        format_removed_table = summary._format_removed_table(removed_table)
        expected_output = (
            '### Removed Tests:\n'
            'No Removed Tests'
        )
        self.assertIn('Removed Tests:',format_removed_table)
        self.assertIn('No Removed Tests',format_removed_table)
        self.assertEqual(expected_output,format_removed_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testFormatRemovedTableHasRemovedTest(self,mock_init_reader):
        """
        Tests formatting remove table when there is removed tests.
        """
        mock_init_reader.return_value = None
        removed_table = [[str(MOCKED_TEST_NAME)]]
        summary = TestHarnessResultsSummary(None)
        format_removed_table = summary._format_removed_table(removed_table)

        expected_table = tabulate(
            removed_table,
            headers=["Test Name"],
            tablefmt="github"
        )
        expected_output = f"### Removed Tests:\n{expected_table}"

        self.assertEqual(expected_output,format_removed_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testFormatAddedTableNoAddedTest(self,mock_init_reader):
        """
        Tests formatting added table when there is no added tests.
        """
        mock_init_reader.return_value = None
        added_table = None
        summary = TestHarnessResultsSummary(None)
        format_added_table = summary._format_added_table(added_table)
        expected_output = (
            '### New Tests:\n'
            'No New Tests'
        )
        self.assertIn('New Tests:',format_added_table)
        self.assertIn('No New Tests',format_added_table)
        self.assertEqual(expected_output,format_added_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testFormatAddedTableHasAddedTest(self,mock_init_reader):
        """
        Tests formatting added table when there is added tests.
        """
        mock_init_reader.return_value = None
        #fake added_table list
        added_table = [[str(MOCKED_TEST_NAME),10]]
        summary = TestHarnessResultsSummary(None)
        format_added_table = summary._format_added_table(added_table)

        expected_table = tabulate(
            added_table,
            headers=["Test Name", "Run Time"],
            tablefmt="github"
        )
        expected_output = f"### New Tests:\n{expected_table}"
        self.assertEqual(expected_output,format_added_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testFormatSameTableNoSameTest(self,mock_init_reader):
        """
        Tests formatting same table when there is no tests.
        """
        mock_init_reader.return_value = None
        same_table = None
        summary = TestHarnessResultsSummary(None)
        format_same_table = summary._format_same_table(same_table)
        expected_output = (
            '### Same Tests that exceed relative run time rate:\n'
            'No Tests'
        )
        self.assertIn('Same Tests',format_same_table)
        self.assertIn('No Tests',format_same_table)
        self.assertEqual(expected_output,format_same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testFormatSameTableHasSameTest(self,mock_init_reader):
        """
        Tests formatting same table when there is same tests with high relative runtime
        """
        mock_init_reader.return_value = None
        same_table =[[str(MOCKED_TEST_NAME),10,17,'70.00%']]
        summary = TestHarnessResultsSummary(None)
        format_same_table = summary._format_same_table(same_table)

        expected_table = tabulate(
            same_table,
            headers=["Test Name", "Base Run Time", "Head Run Time", "Relative Run Time Rate"],
            tablefmt="github"
        )
        expected_output = f"### Same Tests that exceed relative run time rate:\n{expected_table}"

        self.assertEqual(format_same_table, expected_output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testBuildSummaryNoChanges(self,mock_init_reader):
        """
        Tests building a summary when there are no changes between base and head test names.
        """
        mock_init_reader.return_value = None
        summary = TestHarnessResultsSummary(None)
        no_change_buid_summary = summary.build_summary(None,None,None)
        self.assertIn('Removed Tests',no_change_buid_summary)
        self.assertIn('No Removed Tests',no_change_buid_summary)
        self.assertIn('New Tests',no_change_buid_summary)
        self.assertIn('No New Tests',no_change_buid_summary)
        self.assertIn('Same Tests',no_change_buid_summary)
        self.assertIn('No Tests',no_change_buid_summary)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testBuildSummaryHasTests(self,mock_init_reader):
        """
        Tests building a summary when a test are removed from base, newly added and same tests with high runtime
        """
        mock_init_reader.return_value = None
        summary = TestHarnessResultsSummary(None)
        removed_table =[[str(MOCKED_TEST_NAME)]]
        added_table =[[str(MOCKED_TEST_NAME),10]]
        same_table =[[str(MOCKED_TEST_NAME),10,17,'70.00%']]
        has_test_build_summary = summary.build_summary(removed_table,added_table,same_table)
        self.assertIn('Removed Tests',has_test_build_summary)
        self.assertIn('New Tests',has_test_build_summary)
        self.assertIn('Same Tests',has_test_build_summary)
        self.assertIn('Test Name',has_test_build_summary)
        self.assertIn('Run Time',has_test_build_summary)
        self.assertIn(str(MOCKED_TEST_NAME),has_test_build_summary)
        self.assertIn(str(10),has_test_build_summary)
        self.assertIn(str(17),has_test_build_summary)
        self.assertIn('70.00%',has_test_build_summary)

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
        invalid_path = '/this/path/does/not/exist/output.txt'
        stdout = StringIO()
        with redirect_stdout(stdout):
            summary_result = summary.pr(event_id=EVENT_ID, out=invalid_path)

        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.summary_output_file(summary_result, invalid_path)

        output = stdout.getvalue()
        self.assertIn("Failed to write to", output)


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
