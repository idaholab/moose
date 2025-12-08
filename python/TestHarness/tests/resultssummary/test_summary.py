# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultssummary.summary.ResultsSummary."""

import os
import random
from copy import deepcopy
from io import StringIO
from tempfile import NamedTemporaryFile
from typing import Iterable, Optional, Union
from unittest import TestCase, skipUnless

import pytest
from mock import patch
from moosepytest.civet import is_civet_pull_request
from tabulate import tabulate
from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.resultcollection import ResultCollection
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.storedtestresult import StoredTestResult
from TestHarness.resultsstore.utils import TestName
from TestHarness.resultssummary.summary import TestHarnessResultsSummary
from TestHarness.tests.resultsstore.common import random_git_sha, random_version

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = ResultsReader.load_authentication() is not None

# Test database name for testing pull request results
LIVE_DATABASE_NAME = "civet_tests_moose_store_results_live"

# Dummy test name for simple tests
MOCKED_TEST_NAME = TestName("tests/test_harness", "always_ok")

# Fake event ID, or a real one if running on CIVET
EVENT_ID = 1234
if (CIVET_EVENT_ID := os.environ.get("CIVET_EVENT_ID")) is not None:
    EVENT_ID = int(CIVET_EVENT_ID)


class FakeStoredTestResult(StoredTestResult):
    """Fake StoredTestResult for unit testing."""

    def __init__(
        self,
        name: TestName,
        run_time: Optional[float] = 2.0,
        status: Optional[str] = "OK",
        max_memory: Optional[int] = 1234567,
    ):
        """Initialize faked state."""
        self._name = name
        self._run_time: Optional[float] = run_time
        self._status = {"status": status} if status is not None else None
        self._max_memory: Optional[int] = max_memory

    @property
    def run_time(self) -> Optional[float]:
        """Fake run time for a test."""
        return self._run_time

    @property
    def status(self) -> Optional[dict]:
        """Fake status for a test."""
        return self._status

    @property
    def status_value(self) -> Optional[str]:
        """Fake status value for a test."""
        return self.status["status"] if self.status is not None else None

    @property
    def max_memory(self) -> Optional[int]:
        """Fake max memory for a test."""
        return self._max_memory


class FakeStoredResult(StoredResult):
    """Fake StoredResult for unit testing."""

    def __init__(
        self,
        run_time: float = float(random.random()),
        status: Optional[str] = "OK",
        max_memory: int = int(random.random()),
    ):
        """Initialize fake state."""
        self._base_sha: str = random_git_sha()
        self._event_sha: str = random_git_sha()
        self._version: int = random_version()

    @property
    def civet_job_url(self) -> str:
        """Fake CIVET job url for a result."""
        return "foo.com/job"

    @property
    def base_sha(self) -> str:
        """Fake base SHA for a result."""
        return self._base_sha

    @property
    def event_sha(self) -> str:
        """Fake event sha for a result."""
        return self._event_sha

    @property
    def version(self) -> str:
        """Fake event sha for a result."""
        return self._version


class FakeResultCollection(ResultCollection):
    """Fake ResultCollection for unit testing."""

    def __init__(
        self,
        tests: Union[Iterable[FakeStoredTestResult], FakeStoredTestResult],
    ):
        """Initialize fake state."""
        if isinstance(tests, FakeStoredTestResult):
            tests = [tests]
        self._tests: dict[TestName, FakeStoredTestResult] = {
            v.name: deepcopy(v) for v in tests
        }
        self._result = FakeStoredResult()

    def get_test_names(self) -> set[TestName]:
        """Get the underlying test names."""
        return set(list(self._tests.keys()))

    @property
    def result(self) -> FakeStoredResult:
        """Get the underlying faked result."""
        return self._result

    def get_test(self, name: TestName, filters) -> Optional[FakeStoredTestResult]:
        """Get the underlying faked test, if any."""
        test = self._tests.get(name)
        return None if test is None else test


class FakeResultsReader:
    """Fake ResultsReader for unit testing."""

    # List of taked test names
    ALL_FAKE_TEST_NAMES = set(
        [TestName("folder1", "name1"), TestName("folder2", "name2")]
    )

    # A build of all fake tests
    ALL_FAKE_TESTS = [FakeStoredTestResult(v) for v in ALL_FAKE_TEST_NAMES]

    def __init__(
        self,
        no_head: bool = False,
        no_base: bool = False,
        head_tests: Optional[Iterable[FakeStoredTestResult]] = None,
        base_tests: Optional[Iterable[FakeStoredTestResult]] = None,
    ):
        """Initialize fake state."""
        if head_tests is None:
            head_tests = deepcopy(FakeResultsReader.ALL_FAKE_TESTS)
        if base_tests is None:
            base_tests = deepcopy(FakeResultsReader.ALL_FAKE_TESTS)
        self._base_collection = None if no_base else FakeResultCollection(base_tests)
        self._head_collection = None if no_head else FakeResultCollection(head_tests)

    @property
    def head_collection(self) -> FakeResultCollection:
        """Get the head collection (the event collection)."""
        assert self._head_collection is not None
        return self._head_collection

    @property
    def base_collection(self) -> FakeResultCollection:
        """Get the base collection (the old commit collection)."""
        assert self._base_collection is not None
        return self._base_collection

    def get_event_result(self, event_id) -> Optional[FakeResultCollection]:
        """Fake getting the event result (head)."""
        return self._head_collection

    def get_commit_result(self, commit) -> Optional[FakeResultCollection]:
        """Fake getting the commit result (base)."""
        return self._base_collection


class FakeTestHarnessResultsSummary(TestHarnessResultsSummary):
    """Fake TestHarnessResultsSummary for testing with faked results."""

    def __init__(
        self,
        no_head: bool = False,
        no_base: bool = False,
        head_tests: Optional[Iterable[FakeStoredTestResult]] = None,
        base_tests: Optional[Iterable[FakeStoredTestResult]] = None,
    ):
        """Initialize fake state."""
        self.reader: FakeResultsReader = FakeResultsReader(
            no_head=no_head,
            no_base=no_base,
            head_tests=head_tests,
            base_tests=base_tests,
        )


def get_fake_tests(num: int = -1, **kwargs) -> list[FakeStoredTestResult]:
    """
    Get fake tests for unit testing.

    Optional parameters
    -------------------
    num : int
        Max number of tests to get; -1 gets all (default)
    **kwargs :
        Passed to FakeStoredTestResult.

    """
    return [
        FakeStoredTestResult(v, **kwargs)
        for v in deepcopy(FakeResultsReader.ALL_FAKE_TEST_NAMES)
    ][0:num]


def get_fake_test(**kwargs) -> FakeStoredTestResult:
    """
    Get a fake test for unit testing.

    Optional parameters
    -------------------
    **kwargs :
        Passed to FakeStoredTestResult.

    """
    return get_fake_tests(1, **kwargs)[0]


class TestResultsSummary(TestCase):
    """Test TestHarness.resultssummary.summary.ResultsSummary."""

    def setUp(self):
        """Add a patcher for mocking stdout."""
        # Mock stdout so we can more easily get print statements
        self._stdout_patcher = patch("sys.stdout", new=StringIO())
        self.stdout: StringIO = self._stdout_patcher.start()

        # Create a temp file for every test
        self.tmp_file = NamedTemporaryFile()  # noqa: SIM115

    def tearDown(self):
        """Stop the stdout patcher."""
        self._stdout_patcher.stop()

        self.tmp_file.close()

    def test_init(self):
        """Test __init__(), which initializes the reader."""
        with (
            patch.dict(os.environ, {}, clear=True),
            self.assertRaisesRegex(ValueError, "RESULTS_READER"),
        ):
            TestHarnessResultsSummary("unused")

    def test_parse_args(self):
        """Test parse_args()."""
        # Require the database, out file, action
        with self.assertRaises(SystemExit):
            TestHarnessResultsSummary.parse_args([])

    def tests_parse_args_pr(self):
        """Test parse_args() with the pr action."""
        database = "database_name"
        out_file = "file_path"

        # No event id
        base_args = [database, out_file, "pr"]
        with self.assertRaises(SystemExit):
            TestHarnessResultsSummary.parse_args(base_args)

        # All args present, rest default
        base_args += [str(EVENT_ID)]
        args = TestHarnessResultsSummary.parse_args(base_args)
        self.assertEqual(args.database, database)
        self.assertEqual(args.out_file, out_file)
        self.assertEqual(args.event_id, EVENT_ID)
        self.assertEqual(
            args.run_time_floor, TestHarnessResultsSummary.DEFAULT_RUN_TIME_FLOOR
        )
        self.assertEqual(
            args.run_time_rate_floor,
            TestHarnessResultsSummary.DEFAULT_RUN_TIME_RATE_FLOOR,
        )
        self.assertFalse(args.no_run_time_comparison)

        # Set --run-time-floor
        args = TestHarnessResultsSummary.parse_args(
            base_args + ["--run-time-floor=100"]
        )
        self.assertEqual(args.run_time_floor, 100)

        # Set --run-time-rate-floor
        args = TestHarnessResultsSummary.parse_args(
            base_args + ["--run-time-rate-floor=0.1"]
        )
        self.assertEqual(args.run_time_rate_floor, 0.1)

        # Set --no-run-time-comparison
        args = TestHarnessResultsSummary.parse_args(
            base_args + ["--no-run-time-comparison"]
        )
        self.assertTrue(args.no_run_time_comparison)

    def test_pr_tests(self):
        """Tests pr_tests()."""
        summary = FakeTestHarnessResultsSummary()

        base_collection, head_collection = summary.pr_tests(
            EVENT_ID, self.tmp_file.name
        )
        self.assertEqual(base_collection, summary.reader.base_collection)
        self.assertEqual(head_collection, summary.reader.head_collection)

    def test_pr_tests_no_base(self):
        """Test pr_test_names() when no base is available to compare."""
        summary = FakeTestHarnessResultsSummary(no_base=True)
        no_base_results, _ = summary.pr_tests(EVENT_ID, self.tmp_file.name)
        self.assertIsNone(no_base_results)

    def test_pr_tests_no_head(self):
        """Test pr_test_names() when no head results are available."""
        summary = FakeTestHarnessResultsSummary(no_head=True)
        with self.assertRaisesRegex(SystemExit, "Results do not exist for event"):
            summary.pr_tests(EVENT_ID, self.tmp_file.name)

    def test_format_max_memory_None(self):
        """Test _format_max_memory when max_memory is None."""
        test = get_fake_test(max_memory=None)
        max_memory = TestHarnessResultsSummary._format_max_memory(test.max_memory)
        self.assertEqual(max_memory, "0.00")

    def test_format_max_memory(self):
        """Test _format_max_memory when max_memory is None."""
        test = get_fake_test(max_memory=1234567)
        max_memory = TestHarnessResultsSummary._format_max_memory(test.max_memory)
        self.assertEqual(max_memory, "1.23")

    def test_sort_test_time_key_numeric(self):
        """Test that _sort_test_time_key() returns correct key for numeric value."""
        test_table_row = ["`testa.test1`", "4.20"]
        test_time_col_index = 1
        sorting_key = TestHarnessResultsSummary._sort_test_times_key(
            test_table_row, test_time_col_index
        )
        self.assertEqual(sorting_key, (0, -4.20))

    def test_sort_test_time_key_numeric_skip(self):
        """Test that _sort_test_time_key() returns correct key for SKIP."""
        test_table_row = ["`testa.test1`", "SKIP"]
        test_time_col_index = 1
        sorting_key = TestHarnessResultsSummary._sort_test_times_key(
            test_table_row, test_time_col_index
        )
        self.assertEqual(sorting_key, (1, 0))

    def test_sort_test_time_key_numeric_skip_empty(self):
        """Test that _sort_test_time_key() returns correct key for empty string."""
        test_table_row = ["`testa.test1`", ""]
        test_time_col_index = 1
        sorting_key = TestHarnessResultsSummary._sort_test_times_key(
            test_table_row, test_time_col_index
        )
        self.assertEqual(sorting_key, (2, 0))

    def test_sort_times(self):
        """Test sort_test_times() sorting the runtime value, else SKIP."""
        # Mock data for test table
        test_table_row_num_high = ["`testa.test1`", "4.20"]
        test_table_row_num_low = ["`testb.test2`", "1.70"]
        test_table_row_skip = ["`testc.test3`", "SKIP"]
        test_table_row_empty = ["`testd.test4`", ""]
        # Unsorted table
        test_table = [
            test_table_row_skip,
            test_table_row_num_high,
            test_table_row_empty,
            test_table_row_num_low,
        ]
        test_time_col_index = 1

        # Sorted table
        table = TestHarnessResultsSummary.sort_test_times(
            test_table, test_time_col_index
        )
        # Check to ensure the correct sorting sequence
        self.assertEqual(table[0], test_table_row_num_high)
        self.assertEqual(table[1], test_table_row_num_low)
        self.assertEqual(table[2], test_table_row_skip)
        self.assertEqual(table[3], test_table_row_empty)

    def test_build_diff_table(self):
        """Test _build_diff_table() when reporting a test."""
        test = get_fake_test()
        collection = FakeResultCollection(test)
        table = TestHarnessResultsSummary._build_diff_table(
            collection.get_test_names(),
            collection,
        )
        self.assertEqual(len(table), 1)
        self.assertIsInstance(table[0], list)
        self.assertEqual(len(table[0]), 3)
        self.assertIn(str(test.name), table[0][0])
        self.assertEqual(table[0][1], f"{test.run_time:.2f}")
        self.assertEqual(
            table[0][2], TestHarnessResultsSummary._format_max_memory(test.max_memory)
        )

    def test_build_diff_table_no_runtime(self):
        """Test _build_diff_table() with a test without a runtime."""
        test = get_fake_test(run_time=None)
        collection = FakeResultCollection(test)
        table = TestHarnessResultsSummary._build_diff_table(
            collection.get_test_names(),
            collection,
        )
        self.assertEqual(len(table), 1)
        self.assertEqual(len(table[0]), 3)
        self.assertIn(str(test.name), table[0][0])
        self.assertEqual(table[0][1], "")
        self.assertEqual(
            table[0][2], TestHarnessResultsSummary._format_max_memory(test.max_memory)
        )

    def test_build_diff_table_skip(self):
        """Test _build_diff_table() when the status is SKIP."""
        test = get_fake_test(status="SKIP")
        collection = FakeResultCollection(test)
        table = TestHarnessResultsSummary._build_diff_table(
            collection.get_test_names(),
            collection,
        )
        self.assertEqual(len(table), 1)
        self.assertEqual(len(table[0]), 3)
        self.assertIn(str(test.name), table[0][0])
        self.assertEqual(table[0][1], "SKIP")
        self.assertEqual(
            table[0][2], TestHarnessResultsSummary._format_max_memory(test.max_memory)
        )

    def test_build_diff_table_no_maxMemory(self):
        """Test _build_diff_table() with a test without a runtime."""
        test = get_fake_test(max_memory=None)
        collection = FakeResultCollection(test)
        table = TestHarnessResultsSummary._build_diff_table(
            collection.get_test_names(),
            collection,
        )
        self.assertEqual(len(table), 1)
        self.assertEqual(len(table[0]), 3)
        self.assertIn(str(test.name), table[0][0])
        self.assertEqual(table[0][1], f"{test.run_time:.2f}")
        self.assertEqual(table[0][2], "0.00")

    def test_build_same(self):
        """
        Test _build_same() with the same tests when their values are used.

        In this case:
            - the head runtime exceeds a predefined threshold and
            - the relative runtime increase exceeds a defined rate.
        """
        fake_run_time_floor = 1.00
        fake_run_time_rate_floor = 0.50

        # Absolute relative run time rate is higher than
        # the run time floor
        base_test = get_fake_test(run_time=10.0, max_memory=4567890)
        head_test = get_fake_test(run_time=4.0, max_memory=1234567)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        table = TestHarnessResultsSummary._build_same_table(
            head_collection.get_test_names(),
            base_collection,
            head_collection,
            run_time_floor=fake_run_time_floor,
            run_time_rate_floor=fake_run_time_rate_floor,
        )
        assert table is not None
        self.assertEqual(len(table), 1)
        self.assertEqual(len(table[0]), 6)
        self.assertIn(str(base_test.name), table[0][0])
        self.assertEqual(table[0][1], f"{base_test.run_time:.2f}")
        self.assertEqual(table[0][2], f"{head_test.run_time:.2f}")
        self.assertGreater(table[0][2], f"{fake_run_time_floor:.2f}")
        # Compare absolute relative run time rate is higher than floor rate
        self.assertGreater(
            abs(float(table[0][3].strip("%"))), fake_run_time_rate_floor * 100
        )
        self.assertEqual(
            table[0][4],
            TestHarnessResultsSummary._format_max_memory(base_test.max_memory),
        )
        self.assertEqual(
            table[0][5],
            TestHarnessResultsSummary._format_max_memory(head_test.max_memory),
        )

    def test_build_same_zero_base_runtime(self):
        """
        Test _build_same() with the same tests and a zero base runtime.

        The test will not be included in the same table.
        """
        # Base run time is zero
        base_test = get_fake_test(run_time=0.0)
        head_test = get_fake_test(run_time=2.0)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        same = TestHarnessResultsSummary._build_same_table(
            base_collection.get_test_names(),
            base_collection,
            head_collection,
            run_time_floor=1.0,
            run_time_rate_floor=0.5,
        )

        self.assertIsNone(same)

    def test_build_same_no_head_runtime(self):
        """
        Test _build_same() with the same test but without a head run time.

        The test will not be included in same.
        """
        # Head run time is None
        base_test = get_fake_test(run_time=2.0)
        head_test = get_fake_test(run_time=None)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        table = TestHarnessResultsSummary._build_same_table(
            base_collection.get_test_names(),
            base_collection,
            head_collection,
            run_time_floor=1.0,
            run_time_rate_floor=0.5,
        )

        self.assertIsNone(table)

    def test_build_same_no_base_runtime(self):
        """
        Test _build_same() with the same test but without a base run time.

        The test will not be included in same.
        """
        # Base run time is None
        base_test = get_fake_test(run_time=None)
        head_test = get_fake_test(run_time=2.0)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        table = TestHarnessResultsSummary._build_same_table(
            base_collection.get_test_names(),
            base_collection,
            head_collection,
            run_time_floor=1.0,
            run_time_rate_floor=0.5,
        )

        self.assertIsNone(table)

    def test_build_same_base_under(self):
        """
        Test _build_same() with base time is under threshold.

        The test will not be included in same.
        """
        # Base run time is under
        base_test = get_fake_test(run_time=0.5)
        head_test = get_fake_test(run_time=2.0)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        table = TestHarnessResultsSummary._build_same_table(
            base_collection.get_test_names(),
            base_collection,
            head_collection,
            run_time_floor=1.0,
            run_time_rate_floor=0.5,
        )

        self.assertIsNone(table)

    def test_build_same_head_under(self):
        """
        Test _build_same() with head time is under threshold.

        The test will not be included in same.
        """
        # Head run time is under
        base_test = get_fake_test(run_time=2.0)
        head_test = get_fake_test(run_time=0.5)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        table = TestHarnessResultsSummary._build_same_table(
            base_collection.get_test_names(),
            base_collection,
            head_collection,
            run_time_floor=1.0,
            run_time_rate_floor=0.5,
        )

        self.assertIsNone(table)

    def test_build_same_low_relative(self):
        """
        Test _build_same() with relative time under threshold.

        The test will not be included in same.
        """
        # Relative rate is less than 0.5
        run_time_rate_floor = 0.5
        base_test = get_fake_test(run_time=10.0)
        head_test = get_fake_test(run_time=13.0)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)
        self.assertLess(
            TestHarnessResultsSummary.relative_rate(
                head_test.run_time, base_test.run_time
            ),
            run_time_rate_floor,
        )

        table = TestHarnessResultsSummary._build_same_table(
            base_collection.get_test_names(),
            base_collection,
            head_collection,
            run_time_floor=1.0,
            run_time_rate_floor=0.5,
        )

        self.assertIsNone(table)

    def test_diff_table_no_changes(self):
        """Test diff_table() when the test names are the same."""
        base_tests = get_fake_tests()
        head_tests = get_fake_tests()
        base_collection = FakeResultCollection(base_tests)
        head_collection = FakeResultCollection(head_tests)

        removed, added, same = TestHarnessResultsSummary.diff_table(
            base_collection,
            head_collection,
        )
        self.assertIsNone(removed)
        self.assertIsNone(added)
        self.assertIsNone(same)

    def test_diff_table_removed(self):
        """Test diff_table() when a test is removed from the base."""
        base_tests = get_fake_tests(2)
        head_test = get_fake_test()
        base_collection = FakeResultCollection(base_tests)
        head_collection = FakeResultCollection(head_test)
        removed_test = base_tests[1]
        self.assertNotIn(removed_test.name, head_collection.get_test_names())

        removed, added, same = TestHarnessResultsSummary.diff_table(
            base_collection,
            head_collection,
        )

        assert removed is not None
        self.assertEqual(len(removed), 1)
        self.assertIsInstance(removed[0], list)
        self.assertIn(str(removed_test.name), removed[0][0])
        self.assertEqual(removed[0][1], f"{removed_test.run_time:.2f}")
        self.assertIsNone(added)
        self.assertIsNone(same)

    def test_diff_table_added(self):
        """Test diff_table() when a test is added."""
        base_test = get_fake_test()
        head_tests = get_fake_tests(2)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_tests)
        added_test = head_tests[1]
        self.assertNotIn(added_test.name, base_collection.get_test_names())

        removed, added, same = TestHarnessResultsSummary.diff_table(
            base_collection,
            head_collection,
        )

        assert added is not None
        self.assertEqual(len(added), 1)
        self.assertEqual(len(added[0]), 3)
        self.assertIn(str(added_test.name), added[0][0])
        self.assertEqual(added[0][1], f"{added_test.run_time:.2f}")
        self.assertIsNone(removed)
        self.assertIsNone(same)
        self.assertEqual(
            added[0][2],
            TestHarnessResultsSummary._format_max_memory(added_test.max_memory),
        )

    def test_diff_table_same_tests(self):
        """Test diff_table() with the same tests that will be reported."""
        fake_run_time_floor = 1.0
        fake_run_time_rate_floor = 0.5

        base_test = get_fake_test(run_time=10.0, max_memory=1234567)
        head_test = get_fake_test(run_time=17.0, max_memory=4567890)
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        removed, added, same = TestHarnessResultsSummary.diff_table(
            base_collection,
            head_collection,
            run_time_floor=fake_run_time_floor,
            run_time_rate_floor=fake_run_time_rate_floor,
            no_run_time_comparison=False,
        )

        assert same is not None
        self.assertEqual(len(same), 1)
        self.assertEqual(len(same[0]), 6)
        self.assertIn(str(base_test.name), same[0][0])
        self.assertEqual(same[0][1], f"{base_test.run_time:.2f}")
        self.assertEqual(same[0][2], f"{head_test.run_time:.2f}")
        self.assertGreater(same[0][2], f"{fake_run_time_floor:.2f}")
        # Compare absolute relative run time is higher than floor rate
        self.assertGreater(
            abs(float(same[0][3].strip("%"))), fake_run_time_rate_floor * 100
        )
        self.assertEqual(
            same[0][4],
            TestHarnessResultsSummary._format_max_memory(base_test.max_memory),
        )
        self.assertEqual(
            same[0][5],
            TestHarnessResultsSummary._format_max_memory(head_test.max_memory),
        )
        self.assertIsNone(removed)
        self.assertIsNone(added)

    def test_diff_table_no_run_time_comparison(self):
        """Test diff_table() with no_run_time_comparison=True."""
        base_test = get_fake_test()
        head_test = get_fake_test()
        base_collection = FakeResultCollection(base_test)
        head_collection = FakeResultCollection(head_test)

        removed, added, same = TestHarnessResultsSummary.diff_table(
            base_collection,
            head_collection,
            no_run_time_comparison=True,
        )

        self.assertIsNone(same)
        self.assertIsNone(removed)
        self.assertIsNone(added)

    def test_format_table_with_data(self):
        """Test formatting a table when there is data."""
        # Mock the table title, data, header, no_data_message
        base_run_time = 10.0
        head_run_time = 17.0
        test_title = "### Tests with Data:"
        test_table_data = [
            [
                str(MOCKED_TEST_NAME),
                f"{base_run_time:.2f}",
                f"{head_run_time:.2f}",
                "+70.00%",
            ]
        ]
        test_headers = ["Test", "Base (s)", "Head (s)", "+/-"]
        test_no_data_message = "Nome"
        # Expected format table with data
        expected_table = tabulate(
            test_table_data,
            headers=test_headers,
            tablefmt="github",
            disable_numparse=True,
        )
        expected_output = f"{test_title}\n{expected_table}"

        table = TestHarnessResultsSummary._format_table(
            test_title, test_table_data, test_headers, test_no_data_message
        )

        self.assertIn(test_title, table)
        self.assertEqual(table, expected_output)

    def test_format_table_no_data(self):
        """Test formatting a table when there is no data."""
        # Mock the table title, data as None, header, no_data_message
        test_title = "### Tests with No Data:"
        test_table_data = None
        test_headers = ["Test", "Time (s)"]
        test_no_data_message = ""
        # Expected format table with nodata
        expected_output = test_title + "\n" + test_no_data_message

        format_table = TestHarnessResultsSummary._format_table(
            title=test_title,
            table_data=test_table_data,
            headers=test_headers,
            no_data_message=test_no_data_message,
        )

        self.assertIn(test_title, format_table)
        self.assertIn(test_no_data_message, format_table)
        self.assertEqual(expected_output, format_table)

    def test_build_summary_no_changes(self):
        """Test build_summary() with no changes in test names."""
        summary = TestHarnessResultsSummary.build_summary(None, None, None)
        self.assertIn("No change", summary)

    def test_build_summary_no_rmoved(self):
        """Test build_summary() with no removed tests."""
        added = [[str(MOCKED_TEST_NAME), "15.00"]]
        same = [[str(MOCKED_TEST_NAME), "10.00", "17.00", "70.00%"]]
        summary = TestHarnessResultsSummary.build_summary(None, added, same)

        self.assertNotIn("Removed tests", summary)
        self.assertIn("Added tests", summary)
        self.assertIn("Time (s)", summary)
        self.assertIn("15.00", summary)
        self.assertIn("Run time changes", summary)
        self.assertIn("Test", summary)
        self.assertIn("Base (s)", summary)
        self.assertIn("Head (s)", summary)
        self.assertIn("+/-", summary)
        self.assertIn(str(MOCKED_TEST_NAME), summary)
        self.assertIn("10.00", summary)
        self.assertIn("17.00", summary)
        self.assertIn("70.00%", summary)

    def test_build_summary_no_removed_no_added(self):
        """Test build_summary() with no removed or added tests."""
        same = [[str(MOCKED_TEST_NAME), "10.00", "17.00", "70.00%"]]
        summary = TestHarnessResultsSummary.build_summary(None, None, same)

        self.assertNotIn("Removed tests", summary)
        self.assertIn("No added tests", summary)
        self.assertIn("Run time changes", summary)
        self.assertIn("Test", summary)
        self.assertIn("Base (s)", summary)
        self.assertIn("Head (s)", summary)
        self.assertIn("+/-", summary)
        self.assertIn(str(MOCKED_TEST_NAME), summary)
        self.assertIn("10.00", summary)
        self.assertIn("17.00", summary)
        self.assertIn("70.00%", summary)

    def test_build_summary_no_added_no_run_time(self):
        """Test build_summary() with no added tests and no run time changes."""
        removed = [[str(MOCKED_TEST_NAME), "12.00"]]
        summary = TestHarnessResultsSummary.build_summary(removed, None, None)

        self.assertNotIn("Run time changes", summary)
        self.assertIn("Removed tests", summary)
        self.assertIn("No added tests", summary)

    def test_build_summary_has_tests(self):
        """Test build_summary() with removed and added tests and a long shared test."""
        # Mocked table data
        removed = [[str(MOCKED_TEST_NAME), "12.00"]]
        added = [[str(MOCKED_TEST_NAME), "15.00"]]
        same = [[str(MOCKED_TEST_NAME), "10.00", "17.00", "70.00%"]]

        summary = TestHarnessResultsSummary.build_summary(removed, added, same)

        self.assertIn("Removed tests", summary)
        self.assertIn("Added tests", summary)
        self.assertIn("Time (s)", summary)
        self.assertIn("12.00", summary)
        self.assertIn("15.00", summary)
        self.assertIn("Run time changes", summary)
        self.assertIn("Test", summary)
        self.assertIn("Base (s)", summary)
        self.assertIn("Head (s)", summary)
        self.assertIn("+/-", summary)
        self.assertIn(str(MOCKED_TEST_NAME), summary)
        self.assertIn("10.00", summary)
        self.assertIn("17.00", summary)
        self.assertIn("70.00%", summary)

    def test_write_output_valid_file(self):
        """Test write_output() with a valid output file."""
        output_result = "File Path Exist and able to write file"
        TestHarnessResultsSummary.write_output(output_result, self.tmp_file.name)
        # Check output file is able to read
        with open(self.tmp_file.name, "r") as f:
            output = f.read()
            self.assertEqual(output_result, output)

    def test_write_output_invalid_file(self):
        """Test write_output() when the output file path is invalid."""
        output_result = "File Path Exist and able to write file"
        invalid_path = "/this/path/does/not/exist/output.txt"

        TestHarnessResultsSummary.write_output(output_result, invalid_path)

        # Check error message when file path is invalid
        self.assertIn("Failed to write to", self.stdout.getvalue())

    def test_pr_no_changes(self):
        """Test pr() when there are no name changes."""
        summary = FakeTestHarnessResultsSummary()
        summary.pr(event_id=EVENT_ID, out_file=self.tmp_file.name)
        with open(self.tmp_file.name, "r") as f:
            output = f.read()
        self.assertIn("Compared against ", output)
        self.assertIn("No change", output)

    def test_pr_no_base(self):
        """Test pr() without a base."""
        summary = FakeTestHarnessResultsSummary(no_base=True)
        summary.pr(event_id=EVENT_ID, out_file=self.tmp_file.name)

        # Check skip message is displayed in output file
        with open(self.tmp_file.name, "r") as f:
            output = f.read()
        self.assertIn("Base results not available", output)
        self.assertIn(summary.reader.head_collection.result.base_sha[:7], output)

    def test_pr_no_head(self):
        """Test pr() without an head (error)."""
        summary = FakeTestHarnessResultsSummary(no_head=True)

        with self.assertRaisesRegex(SystemExit, "Results do not exist for event"):
            summary.pr(event_id=EVENT_ID, out_file=self.tmp_file.name)

        # Check error message is displayed in output file
        with open(self.tmp_file.name, "r") as f:
            output = f.read()
        self.assertIn("Results do not exist for event", output)

    def test_main_pr(self):
        """Test main() with a pull request."""
        database = "database_name"
        out_file = "out_file_path"
        args = TestHarnessResultsSummary.parse_args(
            [
                database,
                out_file,
                "pr",
                str(EVENT_ID),
            ]
        )

        summary = FakeTestHarnessResultsSummary()
        with patch.object(summary, "pr") as patch_pr:
            summary.main(**vars(args))
        patch_pr.assert_called_once_with(
            event_id=EVENT_ID,
            out_file=out_file,
            run_time_floor=TestHarnessResultsSummary.DEFAULT_RUN_TIME_FLOOR,
            run_time_rate_floor=TestHarnessResultsSummary.DEFAULT_RUN_TIME_RATE_FLOOR,
            no_run_time_comparison=False,
        )

    def test_main_no_action(self):
        """Test main() with an invalid action."""
        with self.assertRaises(NotImplementedError):
            FakeTestHarnessResultsSummary().main(
                action="foo", database="unused", out_file="unused"
            )

    @pytest.mark.live_db
    @skipUnless(
        os.environ.get("TEST_RESULTSSUMMARY"),
        "TEST_RESULTSSUMMARY not set",
    )
    @skipUnless(is_civet_pull_request, "Not a CIVET pull request")
    def test_main_pr_live(self):
        """Tests the main PR action live reading from a live pull request."""
        args = [
            LIVE_DATABASE_NAME,
            self.tmp_file.name,
            "pr",
            str(EVENT_ID),
            "--run-time-floor=0",
            "--run-time-rate-floor=0",
        ]
        parsed = TestHarnessResultsSummary.parse_args(args)
        TestHarnessResultsSummary(LIVE_DATABASE_NAME).main(**vars(parsed))

        with open(self.tmp_file.name, "r") as f:
            output = f.read()

        if "Base results not available" not in output:
            self.assertIn("Compared against", output)
