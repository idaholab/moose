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
    def testPRTestNamesNoChanges(self, mock_get_commit_results,
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
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testBuildRemovedTable(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests  _build_removed_table() when there is removed test
        """
        base_result_with_tests = self.getResult()
        base_test_names = set(base_result_with_tests.test_names)

        mock_get_commit_results.return_value = base_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)
        removed_table = summary._build_removed_table(base_test_names)

        self.assertEqual(len(removed_table), base_result_with_tests.num_tests)
        self.assertIsInstance(removed_table[0], list)
        self.assertIn(str(MOCKED_TEST_NAME), removed_table[0][0])

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    def testBuildAddedTable(self, mock_get_event_results, mock_init_reader):
        """
        Tests _build_added_table() when test is newly added test
        """
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

        summary = TestHarnessResultsSummary(None)
        added_table = summary._build_added_table(
            head_test_names,
            head_result_with_tests)

        self.assertEqual(len(added_table), head_result_with_tests.num_tests)
        self.assertEqual(len(added_table[0]), 2)
        self.assertIn(str(MOCKED_TEST_NAME), added_table[0][0])
        self.assertEqual(added_table[0][1], f'{head_test.run_time:.2f}')

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    def testBuildAddedTableNoRunTime(self, mock_get_event_results, mock_init_reader):
        """
        Tests _build_added_table() when test is newly added test but there is no runtime
        It will put "None" for it's runtime
        """
        head_result_with_tests = self.getResult()
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None
        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)
        head_test._data['timing']['runner_run'] = None

        summary = TestHarnessResultsSummary(None)
        added_table = summary._build_added_table(
            head_test_names,
            head_result_with_tests)

        self.assertEqual(len(added_table), head_result_with_tests.num_tests)
        self.assertEqual(len(added_table[0]), 2)
        self.assertIn(str(MOCKED_TEST_NAME), added_table[0][0])
        self.assertEqual(added_table[0][1], "None")

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
    def BuildSameTableNoHeadRunTime(self, mock_get_commit_results,
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
    def BuildSameTableNoBaseRunTime(self, mock_get_commit_results,
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
    def BuildSameTableLowHeadRunTime(self, mock_get_commit_results,
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
    def BuildSameTableLowRelativeRunTime(self, mock_get_commit_results,
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
        base_test._data['timing']['runner_run'] = 10
        head_test._data['timing']['runner_run'] = 13

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
        self.assertIsNone(added_table)
        self.assertIsNone(same_table)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testDiffTableAddedTest(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests diff_table() when test is newly added.
        """
        base_result_no_tests = self.getResult(no_tests = True)
        head_result_with_tests = self.getResult()
        base_test_no_names = set(base_result_no_tests.test_names)
        head_test_names = set(head_result_with_tests.test_names)

        mock_get_commit_results.return_value = base_result_no_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        summary = TestHarnessResultsSummary(None)
        removed_table, added_table, same_table = summary.diff_table(
            base_result_no_tests,
            head_result_with_tests,
            base_test_no_names,
            head_test_names
        )

        head_test = head_result_with_tests.get_test(MOCKED_TEST_NAME.folder, MOCKED_TEST_NAME.name)

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
        base_test._data['timing']['runner_run'] = 10.00
        head_test._data['timing']['runner_run'] = 17.00

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
        test_title = "### Tests with Data:"
        test_table_data = [[str(MOCKED_TEST_NAME), 10.00, 17.00, '70.00%']]
        test_headers = ["Test Name", "Base Run Time", "Head Run Time", "Relative Run Time Rate"]
        test_no_data_message = "No New Tests"
        # Expected format table with data
        expected_table = tabulate(
            test_table_data,
            headers = test_headers,
            tablefmt = "github"
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
        test_headers = ["Test Name", "Run Time"]
        test_no_data_message = "No New Tests"
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

        self.assertIn('Removed Tests', no_change_buid_summary)
        self.assertIn('No Removed Tests', no_change_buid_summary)
        self.assertIn('New Tests', no_change_buid_summary)
        self.assertIn('No New Tests', no_change_buid_summary)
        self.assertIn('Same Tests', no_change_buid_summary)
        self.assertIn('No Tests', no_change_buid_summary)

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    def testBuildSummaryHasTests(self, mock_init_reader):
        """
        Tests building a summary when a test are removed from base,
        newly added and same tests with high runtime
        """
        mock_init_reader.return_value = None
        # Mocked table data
        removed_table =[[str(MOCKED_TEST_NAME)]]
        added_table =[[str(MOCKED_TEST_NAME), 10.00]]
        same_table =[[str(MOCKED_TEST_NAME), 10.00, 17.00, '70.00%']]

        summary = TestHarnessResultsSummary(None)
        has_test_build_summary = summary.build_summary(removed_table, added_table, same_table)

        self.assertIn('Removed Tests', has_test_build_summary)
        self.assertIn('New Tests', has_test_build_summary)
        self.assertIn('Same Tests', has_test_build_summary)
        self.assertIn('Test Name', has_test_build_summary)
        self.assertIn('Run Time', has_test_build_summary)
        self.assertIn(str(MOCKED_TEST_NAME), has_test_build_summary)
        self.assertIn(str(10), has_test_build_summary)
        self.assertIn(str(17), has_test_build_summary)
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
    def testPRNoChange(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
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
            stdout = StringIO()
            with redirect_stdout(stdout):
                summary_result = summary.pr(
                    event_id = EVENT_ID, out_file = out_file.name
                )
                self.assertIn('Removed Tests:', summary_result)
                self.assertIn('No Removed Tests', summary_result)
                self.assertIn('New Tests:', summary_result)
                self.assertIn('No New Tests', summary_result)
                self.assertIn('Same Tests', summary_result)
                self.assertIn('No Tests', summary_result)

                with open(out_file.name, 'r') as f:
                    output = f.read()
                    self.assertIn('Removed Tests:', output)
                    self.assertIn('No Removed Tests', output)
                    self.assertIn('New Tests:', output)
                    self.assertIn('No New Tests', output)
                    self.assertIn('Same Tests', output)
                    self.assertIn('No Tests', output)

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
            stdout = StringIO()
            with redirect_stdout(stdout):
                summary_result = summary.pr(
                    event_id = EVENT_ID, out_file = out_file.name
                )
                self.assertIsNone(summary_result)
                self.assertIn('Base results not available', stdout.getvalue())
                # Check skip message is displayed in output file
                with open(out_file.name, 'r') as f:
                    output = f.read()
                    self.assertIn('Base results not available', output)

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

    @patch.object(TestHarnessResultsSummary, 'init_reader')
    @patch.object(TestHarnessResultsSummary, 'get_event_results')
    @patch.object(TestHarnessResultsSummary, 'get_commit_results')
    def testMain(self, mock_get_commit_results, mock_get_event_results, mock_init_reader):
        """
        Tests main() output for a PR event, verifying summary messages are printed in output file path.
        """
        base_result_with_tests = self.getResult()
        head_result_with_tests = self.getResult()
        mock_get_commit_results.return_value = base_result_with_tests
        mock_get_event_results.return_value = head_result_with_tests
        mock_init_reader.return_value = None

        with tempfile.NamedTemporaryFile() as out_file:
            summary = TestHarnessResultsSummary(None)
            stdout = StringIO()
            with redirect_stdout(stdout):
                summary.main(out_file = out_file.name, action = 'pr', event_id = EVENT_ID)
                summary_result = stdout.getvalue()
                self.assertIn('Removed Tests:', summary_result)
                self.assertIn('No Removed Tests', summary_result)
                self.assertIn('New Tests:', summary_result)
                self.assertIn('No New Tests', summary_result)
                self.assertIn('Same Tests', summary_result)
                self.assertIn('No Tests', summary_result)

                with open(out_file.name, 'r') as f:
                    output = f.read()
                    self.assertIn('Removed Tests:', output)
                    self.assertIn('No Removed Tests', output)
                    self.assertIn('New Tests:', output)
                    self.assertIn('No New Tests', output)
                    self.assertIn('Same Tests', output)
                    self.assertIn('No Tests', output)

if __name__ == '__main__':
    unittest.main()
