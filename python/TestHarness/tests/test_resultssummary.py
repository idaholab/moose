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
LIVE_DATABASE_NAME = 'civet_tests_moose_store_results_live'
LIVE_TEST_NAME = TestName('tests/test_harness', 'ok')

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

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testPRTestNamesNoChange(self, mock_get_commit_results,
            mock_get_event_results,mock_init_reader):
        """
        Tests pr_test_names() when there are no changes between base and head test names.
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            base_results, head_results, base_test_names, \
                head_test_names = summary.pr_test_names(
                    event_id = EVENT_ID, out_file = out_file.name
            )
            self.assertEqual(base_results, base_result_with_tests)
            self.assertEqual(head_results, head_result_with_tests)
            self.assertEqual(len(base_test_names), base_results.num_tests)
            self.assertEqual(len(head_test_names), head_results.num_tests)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testPRTestNamesNoBaseResults(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests pr_test_names() when no base is available to compare.
        """
        head_result_with_tests = self.getResult()
        # Mock as no base
        mock_get_commit_results.return_value = None
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            stdout = StringIO()
            with redirect_stdout(stdout):
                no_base_results, _, _, _ = summary.pr_test_names(
                    event_id = EVENT_ID, out_file = out_file.name
                )
                self.assertIsNone(no_base_results)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    def testPRTestNamesNoEventResults(self, mock_get_event_results, mock_init_reader):
        """
        Tests pr_test_names() when no event results are available.
        """
        mock_get_event_results.return_value = None
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            with self.assertRaisesRegex(SystemExit, 'Results do not exist for event'):
                summary.pr_test_names(event_id = EVENT_ID, out_file = out_file.name)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testSortTestTimesKeyNumeric(self, mock_init_reader):
        """
        Test _sort_test_time_key() returns correct key for numeric value.
        """
        mock_init_reader.return_value = None
        test_table_row = ['`testa.test1`', '4.20']
        test_time_col_index = 1

        summary = TestHarnessResultsSummary(None)
        sorting_key = summary._sort_test_times_key(test_table_row, test_time_col_index)

        self.assertEqual(sorting_key, (0, -4.20))

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testSortTestTimesKeySKIP(self, mock_init_reader):
        """
        Test _sort_test_time_key() returns correct key for SKIP.
        """
        mock_init_reader.return_value = None
        test_table_row = ['`testa.test1`', 'SKIP']
        test_time_col_index = 1

        summary = TestHarnessResultsSummary(None)
        sorting_key = summary._sort_test_times_key(test_table_row, test_time_col_index)

        self.assertEqual(sorting_key, (1, 0))

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testSortTestTimesKeyEmpty(self, mock_init_reader):
        """
        Test _sort_test_time_key() returns correct key for empty string.
        """
        mock_init_reader.return_value = None
        test_table_row = ['`testa.test1`', '']
        test_time_col_index = 1

        summary = TestHarnessResultsSummary(None)
        sorting_key = summary._sort_test_times_key(test_table_row, test_time_col_index)

        self.assertEqual(sorting_key, (2, 0))

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testSortTestTimes(self, mock_init_reader):
        """
        Tests sort_test_times() to sort with the sequence of runtime value, SKIP then ''
        """
        mock_init_reader.return_value = None
        # Mock data for test table
        test_table_row_num_high = ['`testa.test1`', '4.20']
        test_table_row_num_low = ['`testb.test2`', '1.70']
        test_table_row_skip = ['`testc.test3`', 'SKIP']
        test_table_row_empty = ['`testd.test4`', '']
        # Unsorted table
        test_table = [
            test_table_row_skip,
            test_table_row_num_high,
            test_table_row_empty,
            test_table_row_num_low
        ]
        test_time_col_index = 1

        summary = TestHarnessResultsSummary(None)
        # Sorted table
        test_table = summary.sort_test_times(test_table, test_time_col_index)
        # Check to ensure the correct sorting sequence
        self.assertEqual(test_table[0], test_table_row_num_high)
        self.assertEqual(test_table[1], test_table_row_num_low)
        self.assertEqual(test_table[2], test_table_row_skip)
        self.assertEqual(test_table[3], test_table_row_empty)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildDiffTableRemovedTest(self, mock_get_commit_results, mock_init_reader):
        """
        Tests _build_diff_table() when test is removed
        """
        base_result_with_tests = self.getResult()
        base_test_names = set(base_result_with_tests.test_names)

        mock_get_commit_results.return_value = base_result_with_tests
        mock_init_reader.return_value = None
        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        summary = TestHarnessResultsSummary(None)
        removed_table = summary._build_diff_table(base_test_names,base_result_with_tests)

        self.assertEqual(len(removed_table), base_result_with_tests.num_tests)
        self.assertIsInstance(removed_table[0], list)
        self.assertIn(str(MOCKED_TEST_NAME), removed_table[0][0])
        self.assertEqual(removed_table[0][1], f'{base_test.run_time:.2f}')

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    def testBuildDiffTableAddedTest(self, mock_get_event_results, mock_init_reader):
        """
        Tests _build_diff_table() when test is added
        """
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        summary = TestHarnessResultsSummary(None)
        added_table = summary._build_diff_table(
            head_test_names,
            head_result_with_tests)

        self.assertEqual(len(added_table), head_result_with_tests.num_tests)
        self.assertEqual(len(added_table[0]), 2)
        self.assertIn(str(MOCKED_TEST_NAME), added_table[0][0])
        self.assertEqual(added_table[0][1], f'{head_test.run_time:.2f}')

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    def testBuildDiffTableAddedTestNoRunTime(self, mock_get_event_results, mock_init_reader):
        """
        Tests _build_diff_table() when test is added but there is no runtime
        It will put "" for it's runtime
        """
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test._data['timing']['runner_run'] = None

        summary = TestHarnessResultsSummary(None)
        added_table = summary._build_diff_table(
            head_test_names,
            head_result_with_tests)

        self.assertEqual(len(added_table), head_result_with_tests.num_tests)
        self.assertEqual(len(added_table[0]), 2)
        self.assertIn(str(MOCKED_TEST_NAME), added_table[0][0])
        self.assertEqual(added_table[0][1], "")

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    def testBuildDiffTableStatusValueSKIP(self, mock_get_event_results, mock_init_reader):
        """
        Tests _build_diff_table() when the test result status_value is 'SKIP'
        It will put 'SKIP' for it's runtime
        """
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test._data['status']['status'] = 'SKIP'

        summary = TestHarnessResultsSummary(None)
        added_table = summary._build_diff_table(
            head_test_names,
            head_result_with_tests)

        self.assertEqual(len(added_table), head_result_with_tests.num_tests)
        self.assertEqual(len(added_table[0]), 2)
        self.assertIn(str(MOCKED_TEST_NAME), added_table[0][0])
        self.assertEqual(added_table[0][1], 'SKIP')

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildSameTable(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests _build_same_table() when same test name exit in both base and head, where:
            -the head runtime exceeds a predefined threshold and
            -the relative runtime increase exceeds a defined rate.
        """
        fake_run_time_floor = 1.00
        fake_run_time_rate_floor = 0.50

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock base and head runtime, so that absoluate relative run time rate is higher than fake_run_time_rate_floor
        base_test._data['timing']['runner_run'] = 10.00
        head_test._data['timing']['runner_run'] = 4.0

        summary = TestHarnessResultsSummary(None)
        same_table = summary._build_same_table(
            head_test_names,
            base_result_with_tests,
            head_result_with_tests,
            head_run_time_floor = fake_run_time_floor,
            run_time_rate_floor = fake_run_time_rate_floor
        )

        self.assertEqual(len(same_table), 1)
        self.assertEqual(len(same_table[0]), 4)
        self.assertIn(str(MOCKED_TEST_NAME),same_table[0][0])
        self.assertEqual(same_table[0][1], f'{base_test.run_time:.2f}')
        self.assertEqual(same_table[0][2], f'{head_test.run_time:.2f}')
        self.assertGreater(same_table[0][2], f'{fake_run_time_floor:.2f}')
        # Compare absoulate relative run time rate is higher than floor rate
        self.assertGreater(abs(float(same_table[0][3].strip('%'))), fake_run_time_rate_floor * 100)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildSameTableBaseTimeZero(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests _build_same_table() when same test name exit in both base and head, where:
            -the head runtime exceeds a predefined threshold and
            -the relative runtime increase exceeds a defined rate.
        """
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.5

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock base and head runtime, so that absoluate relative run time rate is higher than fake_run_time_rate_floor
        base_test._data['timing']['runner_run'] = 0.00
        head_test._data['timing']['runner_run'] = 4.0

        summary = TestHarnessResultsSummary(None)
        same_table = summary._build_same_table(
            head_test_names,
            base_result_with_tests,
            head_result_with_tests,
            head_run_time_floor = fake_run_time_floor,
            run_time_rate_floor = fake_run_time_rate_floor
        )

        self.assertEqual(len(same_table), 1)
        self.assertEqual(len(same_table[0]), 4)
        self.assertIn(str(MOCKED_TEST_NAME),same_table[0][0])
        self.assertEqual(same_table[0][1], f'{base_test.run_time:.2f}')
        self.assertEqual(same_table[0][2], f'{head_test.run_time:.2f}')
        self.assertGreater(same_table[0][2], f'{fake_run_time_floor:.2f}')
        # Compare absoulate relative run time rate is higher than floor rate
        self.assertGreater(abs(float(same_table[0][3].strip('%'))), fake_run_time_rate_floor * 100)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildSameTableNoHeadRunTime(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests _build_same_table() when same test name exit in both base and head but
        there is no head run time
        That testname will not be included in same_table
        """
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.5
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock head run time as None
        head_test._data['timing']['runner_run'] = None

        summary = TestHarnessResultsSummary(None)
        same_table = summary._build_same_table(
            head_test_names,
            base_result_with_tests,
            head_result_with_tests,
            head_run_time_floor = fake_run_time_floor,
            run_time_rate_floor = fake_run_time_rate_floor
        )

        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildSameTableNoBaseRunTime(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests _build_same_table() when same test name exit in both base and head but
        there is no base run time
        That testname will not be included in same_table
        """
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.5
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock base run time as None
        base_test._data['timing']['runner_run'] = None

        summary = TestHarnessResultsSummary(None)
        same_table = summary._build_same_table(
            head_test_names,
            base_result_with_tests,
            head_result_with_tests,
            head_run_time_floor = fake_run_time_floor,
            run_time_rate_floor = fake_run_time_rate_floor
        )

        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildSameTableLowHeadRunTime(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests _build_same_table() when same test name exit in both base and head but
        head runtime is lower than threadshold
        That testname will not be included in same_table
        """
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.5
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock head run time is lower than the head run time threadshold (fake_run_time_floor)
        head_test._data['timing']['runner_run'] = 0.5

        summary = TestHarnessResultsSummary(None)
        same_table = summary._build_same_table(
            head_test_names,
            base_result_with_tests,
            head_result_with_tests,
            head_run_time_floor = fake_run_time_floor,
            run_time_rate_floor = fake_run_time_rate_floor
        )

        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildSameTableLowRelativeRunTime(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests _build_same_table() when same test name exit in both base and head and
        relative run time rate is lower than threadshold.
        That testname will not be included in same_table
        """
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.5

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock base and head runtime, so that relative run time rate is less than fake_run_time_rate_floor
        base_test._data['timing']['runner_run'] = 10.00
        head_test._data['timing']['runner_run'] = 13.00

        summary = TestHarnessResultsSummary(None)
        same_table = summary._build_same_table(
            head_test_names,
            base_result_with_tests,
            head_result_with_tests,
            head_run_time_floor = fake_run_time_floor,
            run_time_rate_floor = fake_run_time_rate_floor
        )

        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testDiffTableNoChanges(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when there are no changes between base and head test names.
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        base_test_names = set(base_result_with_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(
            base_result_with_tests,
            head_result_with_tests,
            base_test_names,
            head_test_names
        )
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testDiffTableRemovedTest(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when test is removed from base.
        """
        base_result_with_tests = self.getResult()
        head_result_no_tests = self.getResult(no_tests = True)
        base_test_names = set(base_result_with_tests.test_names)
        head_test_no_names = set(head_result_no_tests.test_names)

        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_no_tests
        mock_init_reader.return_value = None

        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(
            base_result_with_tests,
            head_result_no_tests,
            base_test_names,
            head_test_no_names
        )

        self.assertEqual(len(removed_table), base_result_with_tests.num_tests)
        self.assertIsInstance(removed_table[0], list)
        self.assertIn(str(MOCKED_TEST_NAME), removed_table[0][0])
        self.assertEqual(removed_table[0][1], f'{base_test.run_time:.2f}')
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testDiffTableAddedTest(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when test is added.
        """
        base_result_no_tests = self.getResult(no_tests = True)
        head_result_with_tests = self.getResult()
        base_test_no_names = set(base_result_no_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = base_result_no_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(
            base_result_no_tests,
            head_result_with_tests,
            base_test_no_names,
            head_test_names
        )

        self.assertEqual(len(added_table), head_result_with_tests.num_tests)
        self.assertEqual(len(added_table[0]), 2)
        self.assertIn(str(MOCKED_TEST_NAME), added_table[0][0])
        self.assertEqual(added_table[0][1], f'{head_test.run_time:.2f}')
        self.assertIsNone(removed_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testDiffTableSameTest(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when same test name exit in both base and head, where:
            -the head runtime exceeds a predefined threshold and
            -the relative runtime increase exceeds a defined rate.
        """
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.5

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        base_test_names = set(head_result_with_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock base and head runtime, so that relative run time rate is higher than fake_run_time_rate_floor
        base_test._data['timing']['runner_run'] = 10.0
        head_test._data['timing']['runner_run'] = 17.0

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(
            base_result_with_tests,
            head_result_with_tests,
            base_test_names,
            head_test_names,
            run_time_floor=fake_run_time_floor,
            run_time_rate_floor=fake_run_time_rate_floor,
            no_run_time_comparison = False
        )

        self.assertEqual(len(same_table), 1)
        self.assertEqual(len(same_table[0]), 4)
        self.assertIn(str(MOCKED_TEST_NAME), same_table[0][0])
        self.assertEqual(same_table[0][1], f'{base_test.run_time:.2f}')
        self.assertEqual(same_table[0][2], f'{head_test.run_time:.2f}')
        self.assertGreater(same_table[0][2], f'{fake_run_time_floor:.2f}')
        # Compare absolute relative run time is higher than floor rate
        self.assertGreater(abs(float(same_table[0][3].strip('%'))), fake_run_time_rate_floor * 100)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testDiffTableSameTestDisableRunTimeOption(self, mock_get_commit_results,
            mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when same test name exit in both base and head but
        disable run time comparison
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        base_test_names = set(head_result_with_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = head_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(
            base_result_with_tests,
            head_result_with_tests,
            base_test_names,
            head_test_names,
            no_run_time_comparison = False
        )

        self.assertIsNone(same_table)
        self.assertIsNone(removed_table)
        self.assertIsNone(added_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testFormatTableWithData(self, mock_init_reader):
        """
        Tests formatting table when there is data
        """
        mock_init_reader.return_value = None
        # Mock the table title, data, header, no_data_message
        base_run_time = 10.0
        head_run_time = 17.0
        test_title = "### Tests with Data:"
        test_table_data = [[str(MOCKED_TEST_NAME), f'{base_run_time:.2f}', f'{head_run_time:.2f}', '+70.00%']]
        test_headers = ["Test", "Base (s)", "Head (s)", "+/-"]
        test_no_data_message = "Nome"
        # Expected format table with data
        expected_table = tabulate(
            test_table_data,
            headers = test_headers,
            tablefmt = "github",
            disable_numparse=True
        )
        expected_output = f'{test_title}\n{expected_table}'

        summary = TestHarnessResultsSummary(None)
        format_table = summary._format_table(test_title,test_table_data,test_headers,test_no_data_message)

        self.assertIn(test_title, format_table)
        self.assertEqual(format_table, expected_output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testFormatTableNoData(self, mock_init_reader):
        """
        Tests formatting table when there is no data
        """
        mock_init_reader.return_value = None
        # Mock the table title, data as None, header, no_data_message
        test_title = "### Tests with No Data:"
        test_table_data = None
        test_headers = ["Test", "Time (s)"]
        test_no_data_message = ""
        # Expected format table with nodata
        expected_output = test_title + "\n" + test_no_data_message

        summary = TestHarnessResultsSummary(None)
        format_table = summary._format_table(
            title = test_title,
            table_data = test_table_data,
            headers = test_headers,
            no_data_message = test_no_data_message)

        self.assertIn(test_title, format_table)
        self.assertIn(test_no_data_message, format_table)
        self.assertEqual(expected_output, format_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testBuildSummaryNoChanges(self, mock_init_reader):
        """
        Tests building a summary when there are no changes between base and head test names.
        """
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)
        no_change_buid_summary = summary.build_summary(None, None, None)

        self.assertIn('Removed tests', no_change_buid_summary)
        self.assertIn('Added tests', no_change_buid_summary)
        self.assertIn('Run time changes', no_change_buid_summary)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testBuildSummaryHasTests(self, mock_init_reader):
        """
        Tests building a summary when there are removed test, added test
        and same test with high runtime
        """
        mock_init_reader.return_value = None
        # Mocked table data
        removed_table =[[str(MOCKED_TEST_NAME), '12.00']]
        added_table =[[str(MOCKED_TEST_NAME), '15.00']]
        same_table =[[str(MOCKED_TEST_NAME), '10.00', '17.00', '70.00%']]

        summary = TestHarnessResultsSummary(None)
        has_test_build_summary = summary.build_summary(removed_table, added_table, same_table)

        self.assertIn('Removed tests', has_test_build_summary)
        self.assertIn('Added tests', has_test_build_summary)
        self.assertIn('Time (s)', has_test_build_summary)
        self.assertIn('12.00', has_test_build_summary)
        self.assertIn('15.00', has_test_build_summary)
        self.assertIn('Run time changes', has_test_build_summary)
        self.assertIn('Test', has_test_build_summary)
        self.assertIn('Base (s)', has_test_build_summary)
        self.assertIn('Head (s)', has_test_build_summary)
        self.assertIn('+/-', has_test_build_summary)
        self.assertIn(str(MOCKED_TEST_NAME), has_test_build_summary)
        self.assertIn('10.00', has_test_build_summary)
        self.assertIn('17.00', has_test_build_summary)
        self.assertIn('70.00%', has_test_build_summary)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testWriteOutputFileValidPath(self, mock_init_reader):
        """
        Tests output file write and read when output file path exit
        """
        mock_init_reader.return_value = None
        summary = TestHarnessResultsSummary(None)

        with tempfile.NamedTemporaryFile() as out_file:
            output_result = 'File Path Exist and able to write file'
            summary.write_output(output_result, out_file.name)
            # Check output file is able to read
            with open(out_file.name, 'r') as f:
                output = f.read()
                self.assertEqual(output_result, output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testWriteutputFileInvalidPath(self, mock_init_reader):
        """
        Tests that write_output when output file path is invalid.
        """
        mock_init_reader.return_value = None
        summary = TestHarnessResultsSummary(None)

        output_result = 'File Path Exist and able to write file'
        invalid_path = '/this/path/does/not/exist/output.txt'

        stdout = StringIO()
        with redirect_stdout(stdout):
            summary.write_output(output_result, invalid_path)
            # Check error message when file path is invalid
            output = stdout.getvalue()
            self.assertIn("Failed to write to", output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testPRNoChanges(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests pr() when there are no changes between base and head test names.
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            summary.pr(
                event_id = EVENT_ID, out_file = out_file.name
            )
            with open(out_file.name, 'r') as f:
                output = f.read()
                self.assertIn('Compared against', output)
                self.assertIn('Removed tests', output)
                self.assertIn('Added tests', output)
                self.assertIn('Run time changes', output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testPRNoBase(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests pr() when no base is available to compare.
        """
        head_result_with_tests = self.getResult()
        # Mock as no base
        mock_get_commit_results.return_value = None
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            summary.pr(
                event_id = EVENT_ID, out_file = out_file.name
            )

            # Check skip message is displayed in output file
            with open(out_file.name, 'r') as f:
                output = f.read()
                self.assertIn('Base results not available', output)
                self.assertIn(head_result_with_tests.base_sha[:7], output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    def testPRNoEvent(self, mock_get_event_results, mock_init_reader):
        """
        Tests pr() when there is no event
        """
        mock_get_event_results.return_value = None
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            stdout = StringIO()
            with redirect_stdout(stdout):
                with self.assertRaisesRegex(SystemExit, 'Results do not exist for event'):
                    summary.pr(event_id = EVENT_ID,out_file = out_file.name)
                # Check error message is displayed in output file
                    with open(out_file.name, 'r') as f:
                        output = f.read()
                        self.assertIn('Results do not exist for event', output)

    @unittest.skipUnless(os.environ.get('TEST_RESULTSSUMMARY'), "Skipping because TEST_RESULTSSUMMARY not set")
    @unittest.skipUnless(os.environ.get('CIVET_EVENT_CAUSE', '').startswith('Pull'), 'Skipping because not on a pull request')
    def testPRLive(self):
        """
        Tests pr() to read PR from live database
        """
        # Set the run time and run time rate to zero, so that run time changes show in summary table
        fake_run_time_floor = 0.00
        fake_run_time_rate_floor = 0.00
        # Connect to database and get data from live database
        summary = TestHarnessResultsSummary(LIVE_DATABASE_NAME)
        head_results = summary.get_event_results(EVENT_ID)
        base_results = summary.get_commit_results(head_results.base_sha)
        base_test = base_results.get_test(LIVE_TEST_NAME.folder, LIVE_TEST_NAME.name)
        head_test = head_results.get_test(LIVE_TEST_NAME.folder, LIVE_TEST_NAME.name)

        with tempfile.NamedTemporaryFile() as out_file:
            summary.pr(
                event_id = EVENT_ID,
                out_file = out_file.name,
                run_time_floor = fake_run_time_floor,
                run_time_rate_floor = fake_run_time_rate_floor
            )
            # There is run time changes, so check the format of summary result
            with open(out_file.name, 'r') as f:
                output = f.read()
                self.assertIn('Compared against', output)
                self.assertIn('Removed tests', output)
                self.assertIn('Added tests', output)
                self.assertIn('Run time changes', output)
                self.assertIn('Test', output)
                self.assertIn(str(LIVE_TEST_NAME), output)
                self.assertIn('Base (s)', output)
                self.assertIn(f'{base_test.run_time:.2f}',output)
                self.assertIn('Head (s)', output)
                self.assertIn(f'{head_test.run_time:.2f}',output)
                self.assertIn('+/-', output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testMainNoChanges(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests main() output for a PR event, verifying summary messages are printed in output file path
        when there is no change
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            summary.main(out_file = out_file.name, action = 'pr', event_id = EVENT_ID)
            with open(out_file.name, 'r') as f:
                output = f.read()
                self.assertIn('Compared against', output)
                self.assertIn(base_result_with_tests.event_sha[:7], output)
                self.assertIn(base_result_with_tests.civet_job_url, output)
                self.assertIn('Removed tests', output)
                self.assertIn('Added tests', output)
                self.assertIn('Run time changes', output)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testMainRunTimeChanges(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests main() output for a PR event, verifying summary messages are printed in output file path
        when there is run time changes for same test and absolute relative run time is higher than threadshold
        """
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.3

        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()

        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        base_test = base_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        # Mock base and head runtime, so that abs relative run time rate is higher than fake_run_time_rate_floor
        base_test._data['timing']['runner_run'] = 15.00
        head_test._data['timing']['runner_run'] = 10.00

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            summary.main(out_file = out_file.name,
                action = 'pr',
                event_id = EVENT_ID,
                run_time_floor = fake_run_time_floor,
                run_time_rate_floor = fake_run_time_rate_floor)
            with open(out_file.name, 'r') as f:
                output = f.read()
                self.assertIn('Compared against', output)
                self.assertIn(base_result_with_tests.event_sha[:7], output)
                self.assertIn(base_result_with_tests.civet_job_url, output)
                self.assertIn('Removed tests', output)
                self.assertIn('Added tests', output)
                self.assertIn('Run time changes', output)
                self.assertIn('Test', output)
                self.assertIn('Base (s)', output)
                self.assertIn('Head (s)', output)
                self.assertIn('+/-', output)
                self.assertIn(str(MOCKED_TEST_NAME), output)
                self.assertIn('15.00', output)
                self.assertIn('10.00', output)
                self.assertIn('-33.33%', output)

if __name__ == '__main__':
    unittest.main()
