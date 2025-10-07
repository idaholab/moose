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
from typing import Tuple, Optional

from TestHarness.resultsreader.reader import TestHarnessResultsReader
from TestHarness.resultsreader.results import TestHarnessResults, TestName
from tabulate import tabulate

NoneType = type(None)

class TestHarnessResultsSummary:

    def __init__(self, database: str):
        self.reader = TestHarnessResultsReader(database)

    @staticmethod
    def parseArgs() -> argparse.Namespace:
        """
        Parse command-line arguments for generating a test summary.
        - database : str
            The name of the database.
        - action : str
            The action to perform (e.g., 'pr').
        - event_id : int
            The event ID (required for the 'pr' action).
        """
        parser = argparse.ArgumentParser(description='Produces a summary from test harness results')
        parent = argparse.ArgumentParser(add_help=False)

        parser.add_argument(
            'database',
            type=str,
            help='The name of the database')

        parser.add_argument(
            'out',
            type=str,
            help='path to output the summary to')

        action_parser = parser.add_subparsers(dest='action', help='Action to perform')
        action_parser.required = True

        pr_parser = action_parser.add_parser(
            'pr',
            parents=[parent],
            help='Performs a summary for a pull request'
        )

        pr_parser.add_argument(
            'event_id',
            type=int,
            help='The event ID')

        pr_parser.add_argument(
            '--run-time-floor',
            type=float,
            default=1,
            help='Sets a minimum threshold for the head test run time (seconds)'
        )

        pr_parser.add_argument(
            '--run-time-rate-floor',
            type=float,
            default=0.5,
            help='Sets a minimum relative run time ratio between base and head'
        )

        return parser.parse_args()

    @staticmethod
    def diff_table(base_results: TestHarnessResults, head_results: TestHarnessResults, base_names: set[TestName],
                   head_names: set[TestName], **kwargs) -> Tuple[Optional[list], Optional[list]]:
        """
        Compare test names between the base and current head, and return
        the difference

        Parameters
        ----------
        base_results : TestHarnessResults
            An object that provides access to test results for the head
        head_results : TestHarnessResults
            An object that provides access to test results for the head
        base_names : set of TestName
            The set of test names from the base commit.
        head_names : set of TestName
            The set of test names from the current head commit.

        Optional Parameters
        -------------------
        --run-time-floor : float
            The runtime at which to not check for a difference
            (default: 1 s)
        --run-time-rate-floor : float
            The runtime rate at which to not attach in same_table summary
            (default: 0.5 i.e 50%)

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
        assert isinstance(base_results, TestHarnessResults)
        assert isinstance(head_results, TestHarnessResults)
        assert isinstance(head_names,(set, NoneType))
        assert isinstance(base_names,(set, NoneType))

        head_run_time_floor = kwargs.pop('run_time_floor', 1)
        assert isinstance(head_run_time_floor, (float, int))

        run_time_rate_floor = kwargs.pop('run_time_rate_floor', 0.5)
        assert isinstance(run_time_rate_floor, (float, int))

        removed_names = base_names - head_names
        if removed_names:
            removed_table = list(removed_names)
        else:
            removed_table = None

        add_names = head_names - base_names
        if add_names:
            added_table = []
            for test_name in add_names:
                test_result = head_results.get_test(test_name.folder, test_name.name)
                added_table.append([str(test_name), test_result.run_time])
        else:
            added_table = None

        same_names = base_names & head_names
        if same_names:
            same_table = []
            for test_name in same_names:
                base_result = base_results.get_test(test_name.folder, test_name.name)
                head_result = head_results.get_test(test_name.folder, test_name.name)
                if (head_result.run_time is None or base_result.run_time is None or head_result.run_time < head_run_time_floor):
                    continue
                else:
                    relative_runtime = abs(head_result.run_time - base_result.run_time)/ base_result.run_time
                    if relative_runtime >= run_time_rate_floor:
                        same_table.append([str(test_name), base_result.run_time, head_result.run_time, f'{relative_runtime:.2%}'])
            if not same_table:
                same_table = None
        else:
            same_table = None
        return removed_table, added_table, same_table

    def pr_test_names(self, **kwargs):
        """
        Retrieve test names and base test names for a pull request event.

        This method fetches the test results for a given pull request event ID,
        extracts the test names and base SHA, and retrieves the corresponding
        base test names from the base.

        Parameters
        ----------
        **kwargs : dict
            Keyword arguments. Must include:
            - event_id : int
                The ID of the pull request event.

        Returns
        -------
        base_results : TestHarnessResults
            The test results object for base related to the given event.
        head_results : TestHarnessResults
            The test results object for the given event.
        test_names : set of str
            A set of test names associated with the pull request event.
        base_test_names : set of str or None
            A set of test names from the base commit. Returns `None` if no
            baseline results are found for the base SHA.

        Raises
        ------
        SystemExit
            If no results exist for the given event ID.
        """
        event_id = kwargs.pop('event_id')
        assert isinstance(event_id, int)

        out = kwargs.get('out')

        head_results = self.reader.getEventResults(event_id)
        if head_results is None:
            error_msg = f'ERROR: Results do not exist for event {event_id}'
            self.summary_output_file(error_msg, out)
            raise SystemExit(error_msg)

        test_names = set(head_results.test_names)
        base_sha = head_results.base_sha
        assert isinstance(base_sha, str)

        base_results = self.reader.getCommitResults(base_sha)
        if not isinstance(base_results, TestHarnessResults):
            no_base =f"Comparison not available: no baseline results found for base SHA {base_sha}"
            print(no_base)
            self.summary_output_file(no_base,out)
            return None, head_results, None, test_names
        base_test_names = set(base_results.test_names)
        return base_results, head_results, base_test_names, test_names

    def build_summary(self, removed_table: list, added_table: list, same_table: list) -> str:
        """
        Build a summary report of removed and newly added tests.

        This method generates a formatted string summary that lists tests
        removed from the base and tests newly added in the head, including
        their runtime.

        Parameters
        ----------
        removed_table : list
            A list of removed test names.
        added_table : list
            A list of newly added testnames and its runtime
        same_table : list
            A list of test names that exist in both base and head, where:
            - The head runtime exceeds a predefined threshold.
            - The relative runtime increase exceeds a defined rate.
            and respective runtime and relative runtime rate

        Returns
        -------
        summary : str
            A formatted string summarizing removed and new tests, same tests with xxxx using GitHub-style format
        """
        assert isinstance(removed_table,(list,NoneType))
        assert isinstance(added_table,(list,NoneType))
        assert isinstance(same_table,(list,NoneType))

        summary = []
        summary.append("### Removed Tests:")
        if removed_table:
            table = [[str(test_name)] for test_name in removed_table]
            summary.append(tabulate(table, headers=["Test Name"], tablefmt="github"))
        else:
            summary.append("No Removed Tests")

        summary.append("### New Tests:")
        if added_table:
            summary.append(
                tabulate(
                    added_table,
                    headers=["Test Name", "Run Time"],
                    tablefmt="github"
                )
            )
        else:
            summary.append("No New Tests")

        summary.append(f"### Same Tests that exceed relative run time rate")
        if same_table:
            summary.append(
                tabulate(
                    same_table,
                    headers=["Test Name", "Base Run Time", "Head Run Time", "Relative Run Time Rate"],
                    tablefmt="github"
                )
            )
        else:
            summary.append("No Tests")
        return "\n".join(summary)

    def pr(self, **kwargs) -> str:
        """
        Generate a pull request test summary.

        Parameters
        ----------
        **kwargs : dict
            Keyword arguments passed to `pr_test_names()`, typically including:
            - event_id : int
                The identifier for the PR test event.

        Returns
        -------
        summary_result : str
            A formatted summary string of removed, new, and runtime-sensitive tests.
            If `base_names` is None, returns None.
        """
        out = kwargs.get('out')

        base_results,head_results, base_names, head_names = self.pr_test_names(**kwargs)
        assert isinstance(head_names,set)
        assert isinstance(base_names,(set,NoneType))
        
        if base_names is None:
            return
        removed_table, added_table, same_table = self.diff_table(base_results,head_results, base_names, head_names, **kwargs)
        summary_result = self.build_summary(removed_table, added_table, same_table)
        print(summary_result)
        self.summary_output_file(summary_result,out)
        return summary_result

    def summary_output_file(self, output_result: str, out: str) -> None:
        """
        Write the summary result to a specified output file.

        Parameters
        ----------
        output_result : str
            The formatted summary string to be written to the file.
        out : str
            The file path where the summary should be saved.

        Returns
        -------
        None
            This method does not return anything. It performs a file write operation.
        """
        if output_result is None:
            print("No summary result to write.")
            return

        try:
            with open(out, 'w') as f:
                f.write(output_result)
        except Exception as e:
            print(f"Failed to write to {out}: {e}")

    def main(self, **kwargs):
        action = kwargs['action']
        summary_result = getattr(self, action)(**kwargs)

if __name__ == '__main__':
    args = TestHarnessResultsSummary.parseArgs()
    TestHarnessResultsSummary(args.database).main(**vars(args))

