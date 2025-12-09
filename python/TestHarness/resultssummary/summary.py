#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement the TestHarnessResultSummary for building database summaries."""

import argparse
from typing import List, Optional, Sequence, Tuple

from tabulate import tabulate

from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.resultcollection import ResultCollection
from TestHarness.resultsstore.testdatafilters import TestDataFilter
from TestHarness.resultsstore.utils import TestName

NoneType = type(None)


class TestHarnessResultsSummary:
    """Build a summary for stored test harness results in a database."""

    __test__ = False  # prevents pytest collection

    def __init__(self, database: str):
        """Initialize state given the name of the database."""
        self.reader = ResultsReader(database)

    # Default for --run-time-floor argument
    DEFAULT_RUN_TIME_FLOOR: float = 2.0
    # Default for --run-time-rate-floor argument
    DEFAULT_RUN_TIME_RATE_FLOOR: float = 0.5

    @staticmethod
    def parse_args(args: Sequence[str]) -> argparse.Namespace:
        """Parse command-line arguments."""
        parser = argparse.ArgumentParser(
            description="Produces a summary from test harness results"
        )
        parent = argparse.ArgumentParser(add_help=False)

        parser.add_argument("database", type=str, help="The name of the database")
        parser.add_argument("out_file", type=str, help="path to output the summary to")

        action_parser = parser.add_subparsers(dest="action", help="Action to perform")
        action_parser.required = True

        pr_parser = action_parser.add_parser(
            "pr", parents=[parent], help="Performs a summary for a pull request"
        )

        pr_parser.add_argument("event_id", type=int, help="The event ID")

        pr_parser.add_argument(
            "--run-time-floor",
            type=float,
            default=TestHarnessResultsSummary.DEFAULT_RUN_TIME_FLOOR,
            help="The minimum threshold for a test to checked "
            "for a run time difference",
        )

        pr_parser.add_argument(
            "--run-time-rate-floor",
            type=float,
            default=TestHarnessResultsSummary.DEFAULT_RUN_TIME_RATE_FLOOR,
            help="The ratio by which to report a test's difference in run time",
        )

        pr_parser.add_argument(
            "--no-run-time-comparison",
            action="store_true",
            help="Disable run time comparison",
        )

        return parser.parse_args(args)

    def pr_tests(self, event_id: int, out_file: str) -> Tuple[
        Optional[ResultCollection],
        ResultCollection,
    ]:
        """
        Retrieve test names and base test names for a pull request event.

        This method fetches the test results for a given pull request event ID,
        extracts the test names and base SHA, and retrieves the corresponding
        test names from the base.

        Parameters
        ----------
        event_id : int
            The ID of the pull request event.
        out_file : str
            Path to the output message.

        Returns
        -------
        Optional[ResultCollection]
            The test results object for base related to the given event.
        ResultCollection
            The test results object for the given event.
        Optional[set[TestName]]
            A set of test names from the base commit. Returns `None` if no
            baseline results are found for the base SHA.
        set[TestName]
            A set of test names associated with the pull request event.

        Raises
        ------
        SystemExit
            If no results exist for the given event ID.

        """
        assert isinstance(event_id, int)
        assert isinstance(out_file, str)

        head_collection = self.reader.get_event_result(event_id)
        if head_collection is None:
            error_msg = f"ERROR: Results do not exist for event {event_id}"
            self.write_output(error_msg, out_file)
            raise SystemExit(error_msg)

        base_sha = head_collection.result.base_sha
        assert isinstance(base_sha, str)

        base_collection = self.reader.get_commit_result(base_sha)
        # If no base, display message and return None for base
        if base_collection is None:
            no_base = f"Base results not available for {base_sha[:7]}"
            # Write no base message in out_file path
            self.write_output(no_base, out_file)
            return None, head_collection

        return base_collection, head_collection

    @staticmethod
    def _format_test_name(test_name) -> str:
        """Format a test name for display, wrapped in backticks."""
        return f"`{str(test_name)}`"

    @staticmethod
    def _format_max_memory(max_memory) -> str:
        """Convert max memory of test to Megabyte and 2 dec place."""
        assert isinstance(max_memory, (NoneType, int))
        max_memory_mb = max_memory / 1000000 if max_memory else 0
        max_memory_str = f"{max_memory_mb:.2f}"
        return max_memory_str

    @staticmethod
    def _sort_test_times_key(
        test_table_row: list, test_time_col_index: int
    ) -> Tuple[int, float]:
        """
        Generate a sortable key for each test table row based on test time value.

        Parameters
        ----------
        test_table_row : list
            A list representing a row of data from the test table
        test_time_col_index : int
            The index of the column containing the test time value

        Returns
        -------
        sorting_key : tuple
            A temporary sorting key:
            (0, -value) for numeric values (to sort in descending order)
            (1, 0) for the string 'SKIP'
            (2, 0) for the empty string

        """
        assert isinstance(test_table_row, list)
        assert isinstance(test_time_col_index, int)

        value = test_table_row[test_time_col_index]
        if value.replace(".", "", 1).isdigit():
            return (0, -float(value))
        elif value == "SKIP":
            return (1, 0)
        return (2, 0)

    @staticmethod
    def sort_test_times(test_table: List[List], test_time_col_index: int) -> List[List]:
        """
        Sort a list of test table based on the test time values using custom sort.

        The sorting logic priorities as follow:
            1. Numeric values (sorted in descending order).
            2. The string 'SKIP' (sorted after numeric values).
            3. The empty string (sorted last).

        Parameters
        ----------
        test_table : list[list]
            The test table dataset to sort, where each inner list represents a
            row of test data.
        test_time_col_index : int
            The index of the column containing test time values.

        Returns
        -------
        List[List]
            The sorted test table dataset, ordered according to the custom
            sorting logic.

        """
        assert isinstance(test_table, list)
        assert isinstance(test_time_col_index, int)

        return sorted(
            test_table,
            key=lambda test_table_row: TestHarnessResultsSummary._sort_test_times_key(
                test_table_row, test_time_col_index
            ),
        )

    @staticmethod
    def _build_diff_table(
        test_names: set[TestName], collection: ResultCollection
    ) -> List[List]:
        """
        Build a table of test names that are either removed or added.

        Parameters
        ----------
        test_names : set of TestName
            Set of test names
        collection : ResultCollection
            An object that provides access to test results

        Returns
        -------
        List[List]
            A sorted list of lists where each sublist contains:
                - the formatted test name (str)
                - the run time (str, if numeric value, formatted to 2 decimal places)
            The table is sorted by runtime value,  'SKIP' and empty strings sorted
            accordingly.

        """
        assert isinstance(test_names, set)
        assert isinstance(collection, ResultCollection)

        test_table = []
        for test_name in test_names:
            test = collection.get_test(
                test_name,
                (
                    TestDataFilter.TIMING,
                    TestDataFilter.STATUS,
                    TestDataFilter.MAX_MEMORY,
                ),
            )
            assert test is not None
            # Test is skipped, so show time as SKIP
            if test.status is not None and test.status_value == "SKIP":
                run_time = "SKIP"
            # Test has a run time
            elif test.run_time is not None:
                run_time = f"{test.run_time:.2f}"
            # Test does not have a run time
            else:
                run_time = ""

            test_table.append(
                [
                    TestHarnessResultsSummary._format_test_name(test_name),
                    run_time,
                    TestHarnessResultsSummary._format_max_memory(test.max_memory),
                ]
            )

        # Table will be sorted by runtime value, SKIP then empty
        return TestHarnessResultsSummary.sort_test_times(test_table, 1)

    @staticmethod
    def relative_rate(head_time: float, base_time: float) -> float:
        """Get the relative difference between two times."""
        return (head_time - base_time) / base_time

    @staticmethod
    def _build_same_table(
        same_names: set[TestName],
        base_collection: ResultCollection,
        head_collection: ResultCollection,
        run_time_floor: float,
        run_time_rate_floor: float,
    ) -> Optional[List[List]]:
        """
        Build a table of tests present in both base and head with runtime changes.

        Parameters
        ----------
        same_names : set of TestName
            Set of test names present in both base and head.
        base_collection : ResultCollection
            Object providing access to test results for the base.
        head_collection : ResultCollection
            Object providing access to test results for the head.
        run_time_floor : float
            Minimum runtime in the base and head commit to consider for comparison.
        run_time_rate_floor : float
            Minimum relative runtime change (as a fraction) to include in the table.

        Returns
        -------
        same_table: Optional[List[List]]
            A sorted list of lists based on relative runtime and each sublist contains:
            - Formatted test name (str)
            - Base runtime (str, formatted to 2 decimal places)
            - Head runtime (str, formatted to 2 decimal places)
            - Relative runtime change (str, formatted as percentage with sign)
            Returns None if no tests meet the criteria.

        """
        assert isinstance(same_names, set)
        assert isinstance(base_collection, ResultCollection)
        assert isinstance(head_collection, ResultCollection)
        assert isinstance(run_time_floor, (float, int))
        assert isinstance(run_time_rate_floor, (float, int))

        same_table = []
        for test_name in same_names:
            base_result = base_collection.get_test(
                test_name,
                (
                    TestDataFilter.TIMING,
                    TestDataFilter.STATUS,
                    TestDataFilter.MAX_MEMORY,
                ),
            )
            assert base_result is not None

            head_result = head_collection.get_test(
                test_name,
                (
                    TestDataFilter.TIMING,
                    TestDataFilter.STATUS,
                    TestDataFilter.MAX_MEMORY,
                ),
            )
            assert head_result is not None

            # Skip to check relative run time if run time is None,
            # Zero or below the threshold
            if (
                base_result.run_time is None
                or head_result.run_time is None
                or base_result.run_time == 0
                or head_result.run_time == 0
                or base_result.run_time < run_time_floor
                or head_result.run_time < run_time_floor
            ):
                continue
            # Calculate relative runtime ratio between base and head
            relative_runtime = TestHarnessResultsSummary.relative_rate(
                head_result.run_time, base_result.run_time
            )
            # Check if relative run time rate is higher than threshold,
            # then it will put in the result
            if abs(relative_runtime) >= run_time_rate_floor:
                same_table.append(
                    [
                        TestHarnessResultsSummary._format_test_name(test_name),
                        f"{base_result.run_time:.2f}",
                        f"{head_result.run_time:.2f}",
                        f"{relative_runtime:+.2%}",
                        TestHarnessResultsSummary._format_max_memory(
                            base_result.max_memory
                        ),
                        TestHarnessResultsSummary._format_max_memory(
                            head_result.max_memory
                        ),
                    ]
                )
        if not same_table:
            same_table = None
        else:
            # Sorted based on relative run time rate
            same_table.sort(key=lambda row: float(row[3].strip("%")), reverse=True)
        return same_table

    @staticmethod
    def diff_table(
        base_collection: ResultCollection,
        head_collection: ResultCollection,
        **kwargs,
    ) -> Tuple[Optional[List[List]], Optional[List[List]], Optional[List[List]]]:
        """
        Compare test names between the base and current head.

        Parameters
        ----------
        base_collection : ResultCollection
            An object that provides access to test results for the base
        head_collection : ResultCollection
            An object that provides access to test results for the head

        Optional Parameters
        -------------------
        run-time-floor : float
            The runtime at which to not check for a difference
            (default: 2.00 s)
        run-time-rate-floor : float
            The runtime rate at which to not attach in same_table summary
            (default: 0.50 i.e 50%)
        no-run-time-comparison : bool
            If True, skip runtime comparison
        **kwargs :
            See above.

        Returns
        -------
        Optional[List[List]]
            A sorted list of removed test names and their runtime.
                - the formatted test name (str)
                - the runtime as a string formatted to two decimal places, or "None"
                  if not available.
        Optional[List[List]]
            A sorted list of added test names and their runtime.
                - the formatted test name (str)
                - the runtime as a string formatted to two decimal places, or "None"
                  if not available.
        Optional[List[List]]
            A sorted of tests that exist in both base and head, and their
            optional runtime.
                - Formatted test name
                - Base runtime (str, formatted to 2 decimal places)
                - Head runtime (str, formatted to 2 decimal places)
                - Relative runtime change (str, formatted as percentage with sign)
            Returns None if no-run-time-comparison is True or no tests meet the criteria

        """
        assert isinstance(base_collection, ResultCollection)
        assert isinstance(head_collection, ResultCollection)

        base_names = base_collection.get_test_names()
        head_names = head_collection.get_test_names()

        # Extract optional parameters
        run_time_floor = kwargs.pop("run_time_floor", 2.00)
        assert isinstance(run_time_floor, (float, int))
        run_time_rate_floor = kwargs.pop("run_time_rate_floor", 0.50)
        assert isinstance(run_time_rate_floor, (float, int))
        # Check disable run time comparison option, if True, skip run time comparison
        no_run_time_comparison = kwargs.pop("no_run_time_comparison", False)
        assert isinstance(no_run_time_comparison, bool)

        # Extract removed tests
        removed_names = base_names - head_names
        # Extract added tests
        added_names = head_names - base_names
        # Extract same tests
        same_names = base_names & head_names

        removed_table = (
            TestHarnessResultsSummary._build_diff_table(removed_names, base_collection)
            if removed_names
            else None
        )

        added_table = (
            TestHarnessResultsSummary._build_diff_table(added_names, head_collection)
            if added_names
            else None
        )
        # Check there is same test name and not disable run time comparison option
        if same_names and not no_run_time_comparison:
            same_table = TestHarnessResultsSummary._build_same_table(
                same_names,
                base_collection,
                head_collection,
                run_time_floor,
                run_time_rate_floor,
            )
        else:
            same_table = None
        return removed_table, added_table, same_table

    @staticmethod
    def _format_table(
        title: str,
        table_data: Optional[List[List]],
        headers: List[str],
        no_data_message: str,
    ) -> str:
        """
        Format a GitHub-style table with a section title.

        Parameters
        ----------
        title : str
            The section title to display above the table (e.g., "### Removed tests").
        table_data : list of list or None
            The table content, where each inner list represents a row.
        headers : list of str
            The column headers for the table.
        no_data_message : str
            Message to show when there is no data.

        Returns
        -------
        str
            A formatted string containing the section title and a GitHub-style table,
            or a message indicating there is no tests

        """
        assert isinstance(title, str)
        assert isinstance(table_data, (list, type(None)))
        assert isinstance(headers, list)
        assert isinstance(no_data_message, str)

        formatted_table = [title]
        if table_data:
            assert all(isinstance(row, list) for row in table_data)
            formatted_table.append(
                tabulate(
                    table_data,
                    headers=headers,
                    tablefmt="github",
                    disable_numparse=True,
                )
            )
        else:
            formatted_table.append(no_data_message)

        return "\n".join(formatted_table)

    @staticmethod
    def build_summary(
        removed_table: Optional[list],
        added_table: Optional[list],
        same_table: Optional[list],
    ) -> str:
        """
        Build a summary report of removed, added, and same tests with optional runtimes.

        Parameters
        ----------
        removed_table : Optional[list]
            A list of removed test name, each containing a formatted test name.
        added_table : Optional[list]
            A sorted list of added test names and their optional runtime.
                - the formatted test name (str)
                - the runtime as a string formatted to two decimal places, or "None"
                  if not available.
        same_table : Optional[list]
            A sorted of tests that exist in both base and head, and their
            optional runtime.
                - Formatted test name
                - Base runtime (str, formatted to 2 decimal places)
                - Head runtime (str, formatted to 2 decimal places)
                - Relative runtime change (str, formatted as percentage with sign)

        Returns
        -------
        str
            A formatted string using GitHub-style markdown that summarizes:
                - Removed tests
                - Added tests with runtime
                - Same tests with high relative runtime rate
            If all table is None, 'No change' will display.
            If one or more table is available:
                - there is no removed_table, Removed tests portion won't display
                - there is no same_table, Run time changes portion won't display
                - there is no added_table, 'No added tests' will display

        """
        assert isinstance(removed_table, (list, NoneType))
        assert isinstance(added_table, (list, NoneType))
        assert isinstance(same_table, (list, NoneType))

        summary = []
        # All table are none, display no change
        if removed_table is None and added_table is None and same_table is None:
            summary.append("\nNo change\n")
        else:
            # Format removed table
            if removed_table:
                summary.append(
                    TestHarnessResultsSummary._format_table(
                        "\n### Removed tests\n",
                        removed_table,
                        ["Test", "Time (s)", "MEM (MB)"],
                        "",
                    )
                )
            # Format added table
            if added_table:
                summary.append(
                    TestHarnessResultsSummary._format_table(
                        "\n### Added tests\n",
                        added_table,
                        ["Test", "Time (s)", "MEM (MB)"],
                        "",
                    )
                )
            else:
                summary.append("\n### No added tests\n")
            # Format same table
            if same_table:
                summary.append(
                    TestHarnessResultsSummary._format_table(
                        "\n### Run time changes\n",
                        same_table,
                        [
                            "Test",
                            "Base (s)",
                            "Head (s)",
                            "+/-",
                            "Base (MB)",
                            "Head (MB)",
                        ],
                        "",
                    )
                )
        return "\n".join(summary)

    @staticmethod
    def write_output(output_result: str, out_file: str) -> None:
        """
        Write the summary result to a specified output file.

        Parameters
        ----------
        output_result : str
            The formatted summary string to be written to the file.
        out_file : str
            The file path where the summary should be saved.

        """
        try:
            with open(out_file, "w") as f:
                f.write(output_result)
        except Exception as e:
            print(f"Failed to write to {out_file}: {e}")

    def pr(self, event_id: int, out_file: str, **kwargs) -> Optional[str]:
        """
        Generate a pull request test summary.

        Parameters
        ----------
        event_id : int
            The identifier for the PR test event.
        out_file : str
            path to output the summary to
        **kwargs : dict
            Keyword arguments passed to `diff_table()`

        Returns
        -------
        Optional[str]
            A formatted summary string of removed, added, and runtime-sensitive tests.
            If `base_names` is None, returns None.

        """
        assert isinstance(event_id, int)
        assert isinstance(out_file, str)

        base_collection, head_collection = self.pr_tests(event_id, out_file)

        assert isinstance(base_collection, (ResultCollection, NoneType))
        assert isinstance(head_collection, ResultCollection)

        if base_collection is None:
            return

        removed_table, added_table, same_table = self.diff_table(
            base_collection, head_collection, **kwargs
        )
        # Display base commit and url
        base_result = base_collection.result
        base_sha = base_result.event_sha[:7]
        url = base_result.civet_job_url
        summary_result = f"Compared against {base_sha} in job [{url}](https://{url}).\n"
        summary_result += self.build_summary(removed_table, added_table, same_table)
        # Write results in out_file path
        self.write_output(summary_result, out_file)
        return summary_result

    def main(self, **kwargs):
        """Perform the main action; run from __main__."""
        action = kwargs.pop("action")
        kwargs.pop("database")
        out_file = kwargs.pop("out_file")

        if action == "pr":
            self.pr(
                event_id=kwargs.pop("event_id"),
                out_file=out_file,
                **kwargs,
            )
        else:
            raise NotImplementedError(f"Action {action} not implemented")


if __name__ == "__main__":  # pragma: no cover
    from sys import argv

    args = TestHarnessResultsSummary.parse_args(argv[1:])
    TestHarnessResultsSummary(args.database).main(**vars(args))
