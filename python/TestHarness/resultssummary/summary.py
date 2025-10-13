#!/usr/bin/env python3

#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import argparse
from typing import Tuple, Optional, List

from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.storedresults import StoredResult, TestName
from tabulate import tabulate

NoneType = type(None)

class TestHarnessResultsSummary:
    def __init__(self, database: str):
        self.reader = self.init_reader(database)

    def init_reader(self, database: str) -> ResultsReader:
        return ResultsReader(database)

    @staticmethod
    def parseArgs() -> argparse.Namespace:
        """
        Parse command-line arguments for generating a test summary.
        """
        parser = argparse.ArgumentParser(description = 'Produces a summary from test harness results')
        parent = argparse.ArgumentParser(add_help = False)

        parser.add_argument(
            'database',
            type = str,
            help = 'The name of the database')

        parser.add_argument(
            'out_file',
            type = str,
            help = 'path to output the summary to')

        action_parser = parser.add_subparsers(dest = 'action', help = 'Action to perform')
        action_parser.required = True

        pr_parser = action_parser.add_parser(
            'pr',
            parents = [parent],
            help = 'Performs a summary for a pull request'
        )

        pr_parser.add_argument(
            'event_id',
            type = int,
            help = 'The event ID')

        pr_parser.add_argument(
            '--run-time-floor',
            type = float,
            default = 1,
            help = 'Sets a minimum threshold for the head test run time (seconds)'
        )

        pr_parser.add_argument(
            '--run-time-rate-floor',
            type = float,
            default = 0.5,
            help = 'Sets a minimum relative run time ratio between base and head'
        )

        pr_parser.add_argument(
            '--no-run-time-comparison', action='store_true', help='Disable run time comparison')

        return parser.parse_args()

    def get_commit_results(self, commit: str) -> Optional[StoredResult]:
        """
        The results associated with a commit.

        This is a separate function so that it can be
        mocked in unit tests
        """
        return self.reader.getCommitResults(commit)

    def get_event_results(self, event_id: int) -> Optional[StoredResult]:
        """
        The results associated with a event_id.

        This is a separate function so that it can be
        mocked in unit tests
        """
        return self.reader.getEventResults(event_id)

    def pr_test_names(self, event_id: int, out_file: str) -> Tuple[Optional[StoredResult], Optional[StoredResult],
            Optional[set], Optional[set]]:
        """
        Retrieve test names and base test names for a pull request event.

        This method fetches the test results for a given pull request event ID,
        extracts the test names and base SHA, and retrieves the corresponding
        base test names from the base.

        Parameters
        ----------
        event_id : int
            The ID of the pull request event.
        out_file : str
            path to output message

        Returns
        -------
        base_results : StoredResult
            The test results object for base related to the given event.
        head_results : StoredResult
            The test results object for the given event.
        base_test_names : set of str or None
            A set of test names from the base commit. Returns `None` if no
            baseline results are found for the base SHA.
        test_names : set of str
            A set of test names associated with the pull request event.

        Raises
        ------
        SystemExit
            If no results exist for the given event ID.
        """
        assert isinstance(event_id, int)
        assert isinstance(out_file, str)

        head_results = self.get_event_results(event_id)
        if head_results is None:
            error_msg = f'ERROR: Results do not exist for event {event_id}'
            self.write_output(error_msg, out_file)
            raise SystemExit(error_msg)

        test_names = set(head_results.test_names)
        base_sha = head_results.base_sha
        assert isinstance(base_sha, str)

        base_results = self.get_commit_results(base_sha)
        #if no base, display message and return None for base
        if not isinstance(base_results, StoredResult):
            no_base = f"Base results not available for {base_sha}"
            print(no_base)
            self.write_output(no_base, out_file)
            return None, head_results, None, test_names

        base_test_names = set(base_results.test_names)
        return base_results, head_results, base_test_names, test_names

    @staticmethod
    def _format_test_name(test_name):
        """
        Format a test name for display.

        Parameters
        ----------
        test_name : TestName
            The test name object to format.

        Returns
        -------
        str
            A string representation of the test name, wrapped in backticks.
        """
        return f'`{str(test_name)}`'

    def _build_removed_table(self, removed_names: set[TestName]) -> Optional[list]:
        """
        Build a table of removed test names.

        Parameters
        ----------
        removed_names : set of TestName
            Set of test names that were present in the base but not in the head.

        Returns
        -------
        list of list
            A list of lists, each containing a formatted test name.
        """
        assert isinstance(removed_names, set)
        return [[self._format_test_name(test_name)] for test_name in removed_names]

    def _build_added_table(self, add_names: set[TestName], head_results: StoredResult,
            no_run_time_comparison: bool) -> Optional[list]:
        """
        Build a table of newly added test names, optionally including runtime.

        Parameters
        ----------
        add_names : set of TestName
            Set of test names that are present in the head but not in the base.
        head_results : StoredResult
            An object that provides access to test results for the head
        no_run_time_comparison : bool
            If True, skip runtime comparison and only include test names.

        Returns
        -------
        list of list or None
            A list of lists containing formatted test names and optionally their runtime.
            Returns None if no valid runtimes are found.
        """
        assert isinstance(add_names, set)
        assert isinstance(head_results, StoredResult)
        assert isinstance(no_run_time_comparison, bool)
        added_table = []
        if no_run_time_comparison:
            #no run time
            added_table = [[self._format_test_name(test_name)] for test_name in add_names]
        else:
            #with run time
            for test_name in add_names:
                test_result = head_results.get_test(test_name.folder, test_name.name)
                if  test_result.run_time is None:
                    continue
                added_table.append([
                    self._format_test_name(test_name),
                    f'{test_result.run_time:.2f}',
                ])
            if not added_table:
                added_table = None
            else:
                #sorted based on run time
                added_table.sort(key=lambda row: float(row[1]), reverse=True)
        return added_table

    def _build_same_table(self, same_names: set[TestName],
            base_results: StoredResult, head_results: StoredResult,
            head_run_time_floor: float, run_time_rate_floor: float)-> Optional[list]:
        """
        Build a table of tests present in both base and head with significant runtime changes.

        Parameters
        ----------
        same_names : set of TestName
            Set of test names present in both base and head.
        base_results : StoredResult
            Object providing access to test results for the base.
        head_results : StoredResult
            Object providing access to test results for the head.
        head_run_time_floor : float
            Minimum runtime in the head commit to consider for comparison.
        run_time_rate_floor : float
            Minimum relative runtime change (as a fraction) to include in the table.

        Returns
        -------
        list of list or None
            A sorted list of lists based on relative runtime containing:
            - Formatted test name
            - Base runtime (str, formatted to 2 decimal places)
            - Head runtime (str, formatted to 2 decimal places)
            - Relative runtime change (str, formatted as percentage with sign)
            Returns None if no tests meet the criteria.
        """
        assert isinstance(same_names, set)
        assert isinstance(base_results, StoredResult)
        assert isinstance(head_results, StoredResult)
        assert isinstance(head_run_time_floor, (float, int))
        assert isinstance(run_time_rate_floor, (float, int))

        same_table = []
        #disable run time comparison
        for test_name in same_names:
            base_result = base_results.get_test(test_name.folder, test_name.name)
            head_result = head_results.get_test(test_name.folder, test_name.name)
            #Skip to check relative run time if run time is None or below the threadshold
            if  head_result.run_time is None or \
                base_result.run_time is None or \
                head_result.run_time < head_run_time_floor:
                continue
            #Calculate relative runtime ratio between base and head
            relative_runtime = (head_result.run_time - base_result.run_time) / base_result.run_time
            #Check if relative run time rate is higher than threadshold, then it will put in the result
            if abs(relative_runtime) >= run_time_rate_floor:
                same_table.append(
                    [
                        self._format_test_name(test_name),
                        f'{base_result.run_time:.2f}',
                        f'{head_result.run_time:.2f}',
                        f'{relative_runtime:+.2%}'
                    ]
                )
        if not same_table:
            same_table = None
        else:
            #sorted based on relative run time rate
            same_table.sort(key=lambda row: float(row[3].strip('%')), reverse=True)
        return same_table

    def diff_table(self, base_results: StoredResult, head_results: StoredResult, base_names: set[TestName],
            head_names: set[TestName], **kwargs) -> Tuple[Optional[list], Optional[list], Optional[list]]:
        """
        Compare test names between the base and current head, and return
        the difference

        Parameters
        ----------
        base_results : StoredResult
            An object that provides access to test results for the base
        head_results : StoredResult
            An object that provides access to test results for the head
        base_names : set of TestName
            The set of test names from the base commit.
        head_names : set of TestName
            The set of test names from the current head commit.

        Optional Parameters
        -------------------
        run-time-floor : float
            The runtime at which to not check for a difference
            (default: 1 s)
        run-time-rate-floor : float
            The runtime rate at which to not attach in same_table summary
            (default: 0.5 i.e 50%)
        no-run-time-comparison
            if has this parameter, skip run time comparison
        Returns
        -------
        removed_table : list or None
            A list of test names that were present in the base but not in the head.
        added_table : list or None
            A list of newly added test names along with their runtime.
        same_table : list or None
            A list of test names, runtime and relative runtime rate that exist in both base and head, where:
            - The head runtime exceeds a predefined threshold (run-time-floor).
            - The relative runtime increase exceeds a defined rate (run-time-rate-floor).
        """
        assert isinstance(base_results, StoredResult)
        assert isinstance(head_results, StoredResult)
        assert isinstance(head_names,(set, NoneType))
        assert isinstance(base_names,(set, NoneType))

        #Extract potional parameters
        head_run_time_floor = kwargs.pop('run_time_floor', 1.0)
        assert isinstance(head_run_time_floor, (float, int))
        run_time_rate_floor = kwargs.pop('run_time_rate_floor', 0.5)
        assert isinstance(run_time_rate_floor, (float, int))
        #check disable run time comparison option, it will display only test name and skip run time
        no_run_time_comparison = kwargs.pop('no_run_time_comparison', False)

        #Extract removed tests
        removed_names = base_names - head_names
        #Extract added tests
        added_names = head_names - base_names
        #Extract same tests
        same_names = base_names & head_names

        removed_table = self._build_removed_table(removed_names) if removed_names else None

        added_table = self._build_added_table(
            added_names,
            head_results,
            no_run_time_comparison) if added_names else None
        # Check there is same test name and not disable run time comparison option
        if same_names and not no_run_time_comparison:
            same_table = self._build_same_table(
                same_names,
                base_results,
                head_results,
                head_run_time_floor,
                run_time_rate_floor)
        else:
            same_table = None
        return removed_table, added_table, same_table

    @staticmethod
    def _format_table(title: str, table_data: Optional[List[List]], headers: List[str], no_data_message: str) -> str:
        """
        Format a GitHub-style table with a section title.
        Parameters
        ----------
        title : str
            The section title to display above the table (e.g., "### Removed Tests:").
        table_data : list of list or None
            The table content, where each inner list represents a row.
        headers : list of str
            The column headers for the table.

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
            formatted_table.append(tabulate(table_data, headers=headers, tablefmt="github"))
        else:
            formatted_table.append(no_data_message)

        return "\n".join(formatted_table)

    def build_summary(self, removed_table: list, added_table: list, same_table: list) -> str:
        """
        Build a summary report of removed, newly added tests, same test and Optional (Runtime result)

        Parameters
        ----------
        removed_table : list
            A list of removed test names.
        added_table : list
            A list of newly added testnames and its runtime
        same_table : list
            A list of test names, runtime and relative runtime rate that exist in both base and head, where:
            - The head runtime exceeds a predefined threshold (run-time-floor).
            - The relative runtime increase exceeds a defined rate (run-time-rate-floor).

        Returns
        -------
        summary : str
            A formatted string summarizing removed and new tests,
            same tests with high relative runtime rate
            using GitHub-style format
        """
        assert isinstance(removed_table, (list, NoneType))
        assert isinstance(added_table, (list, NoneType))
        assert isinstance(same_table, (list, NoneType))

        summary = []
        #Format removed table
        summary.append(
            self._format_table(
                "### Removed Tests:",
                removed_table,
                ["Test Name"],
                "No Removed Tests"
            )
        )
        #Format added table
        summary.append(
            self._format_table(
                "### New Tests:",
                added_table,
                ["Test Name", "Run Time"],
                "No New Tests"
            )
        )
        #Format same table
        summary.append(
            self._format_table(
                "### Same Tests that exceed relative run time rate:",
                same_table,
                ["Test Name", "Base Run Time", "Head Run Time", "Relative Run Time Rate"],
                "No Tests"
            )
        )
        return "\n".join(summary)

    def write_output(self, output_result: str, out_file: str) -> None:
        """
        Write the summary result to a specified output file.

        Parameters
        ----------
        output_result : str
            The formatted summary string to be written to the file.
        out file : str
            The file path where the summary should be saved.

        Returns
        -------
        None
            This method does not return anything. It performs a file write operation.
        """
        try:
            with open(out_file, 'w') as f:
                f.write(output_result)
        except Exception as e:
            print(f"Failed to write to {out_file}: {e}")

    def pr(self, event_id: int, out_file :str, **kwargs) -> str:
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
        summary_result : str
            A formatted summary string of removed, new, and runtime-sensitive tests.
            If `base_names` is None, returns None.
        """
        assert isinstance(event_id, int)
        assert isinstance(out_file, str)

        base_results,head_results, base_names, head_names = self.pr_test_names(event_id, out_file)

        assert isinstance(base_results, (StoredResult, NoneType))
        assert isinstance(head_results, StoredResult)
        assert isinstance(base_names, (set, NoneType))
        assert isinstance(head_names, set)

        if base_results is None:
            return

        removed_table, added_table, same_table = self.diff_table(
            base_results,
            head_results,
            base_names,
            head_names,
            **kwargs
        )
        summary_result = self.build_summary(removed_table, added_table, same_table)
        print(summary_result)
        self.write_output(summary_result, out_file)
        return summary_result

    def main(self, **kwargs):
        action = kwargs.pop('action')

        if action == 'pr':
            self.pr(
                event_id = kwargs.pop('event_id'),
                out_file = kwargs.pop('out_file'),
                **kwargs
            )
        else:
            getattr(self, action)(**kwargs)

if __name__ == '__main__':
    args = TestHarnessResultsSummary.parseArgs()
    TestHarnessResultsSummary(args.database).main(**vars(args))

