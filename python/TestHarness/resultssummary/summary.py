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

    def pr_test_names(self, event_id:int, out:str) -> Tuple[Optional[StoredResult],Optional[StoredResult],
                                                            Optional[set],Optional[set]]:
        """
        Retrieve test names and base test names for a pull request event.

        This method fetches the test results for a given pull request event ID,
        extracts the test names and base SHA, and retrieves the corresponding
        base test names from the base.

        Parameters
        ----------
        event_id : int
            The ID of the pull request event.
        out : str
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
        assert isinstance(out, str)

        head_results = self.get_event_results(event_id)
        if head_results is None:
            error_msg = f'ERROR: Results do not exist for event {event_id}'
            self.write_output(error_msg, out)
            raise SystemExit(error_msg)

        test_names = set(head_results.test_names)
        base_sha = head_results.base_sha
        assert isinstance(base_sha, str)

        base_results = self.get_commit_results(base_sha)
        if not isinstance(base_results, StoredResult):
            no_base =f"Base results not available for {base_sha}"
            print(no_base)
            self.write_output(no_base,out)
            return None, head_results, None, test_names
        base_test_names = set(base_results.test_names)
        return base_results, head_results, base_test_names, test_names

    @staticmethod
    def diff_table(base_results: StoredResult, head_results: StoredResult, base_names: set[TestName],
                   head_names: set[TestName], **kwargs) -> Tuple[Optional[list], Optional[list],Optional[list]]:
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

        head_run_time_floor = kwargs.pop('run_time_floor', 1)
        assert isinstance(head_run_time_floor, (float, int))

        run_time_rate_floor = kwargs.pop('run_time_rate_floor', 0.5)
        assert isinstance(run_time_rate_floor, (float, int))

        removed_names = base_names - head_names
        removed_table = [str(v) for v in removed_names] if removed_names else None

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
                if (head_result.run_time is None or
                    base_result.run_time is None or
                    head_result.run_time < head_run_time_floor):
                    continue
                else:
                    relative_runtime = abs(head_result.run_time - base_result.run_time) / base_result.run_time
                    if relative_runtime >= run_time_rate_floor:
                        same_table.append([str(test_name),
                                           base_result.run_time,
                                           head_result.run_time,
                                           f'{relative_runtime:.2%}'
                                           ])
            if not same_table:
                same_table = None
        else:
            same_table = None
        return removed_table, added_table, same_table

    def _format_removed_table(self, removed_table: list) -> str:
        """
        Formatting GitHub-style table for removed test results
        """
        assert isinstance(removed_table,(list,NoneType))
        format_removed_table = ["### Removed Tests:"]
        if removed_table:
            format_removed_table.append(tabulate(removed_table, headers=["Test Name"], tablefmt="github"))
        else:
            format_removed_table.append("No Removed Tests")
        return "\n".join(format_removed_table)

    def _format_added_table(self, added_table: list) -> str:
        """
        Formatting GitHub-style table for added test results
        """
        assert isinstance(added_table,(list,NoneType))
        format_added_table = ["### New Tests:"]
        if added_table:
            format_added_table.append(
                tabulate(
                    added_table,
                    headers=["Test Name", "Run Time"],
                    tablefmt="github"
                )
            )
        else:
            format_added_table.append("No New Tests")
        return "\n".join(format_added_table)

    def _format_same_table(self, same_table: list) -> str:
        """
        Formatting GitHub-style table for same test results that exceed relative run time rate
        """
        assert isinstance(same_table,(list,NoneType))
        format_same_table = ["### Same Tests that exceed relative run time rate:"]
        if same_table:
            format_same_table.append(
                tabulate(
                    same_table,
                    headers=["Test Name",
                             "Base Run Time",
                             "Head Run Time",
                             "Relative Run Time Rate"],
                    tablefmt="github"
                )
            )
        else:
            format_same_table.append("No Tests")
        return "\n".join(format_same_table)

    def build_summary(self, removed_table: list, added_table: list, same_table: list) -> str:
        """
        Build a summary report of removed, newly added tests, same test with high relative runtime rate

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
        assert isinstance(removed_table,(list,NoneType))
        assert isinstance(added_table,(list,NoneType))
        assert isinstance(same_table,(list,NoneType))

        summary = []
        summary.append(self._format_removed_table(removed_table))
        summary.append(self._format_added_table(added_table))
        summary.append(self._format_same_table(same_table))

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

    def pr(self, event_id: int, out:str, **kwargs) -> str:
        """
        Generate a pull request test summary.

        Parameters
        ----------
        event_id : int
                The identifier for the PR test event.
        out : str
                path to output the summary to
        **kwargs : dict
            Keyword arguments passed to `diff_table()`

        Returns
        -------
        summary_result : str
            A formatted summary string of removed, new, and runtime-sensitive tests.
            If `base_names` is None, returns None.
        """
        assert isinstance(event_id,int)
        assert isinstance(out,str)

        base_results,head_results, base_names, head_names = self.pr_test_names(event_id, out)
        assert isinstance(base_results,(StoredResult,NoneType))
        assert isinstance(head_results, StoredResult)
        assert isinstance(base_names,(set,NoneType))
        assert isinstance(head_names,set)

        if base_results is None:
            return
        removed_table, added_table, same_table = self.diff_table(base_results,
                                                                 head_results,
                                                                 base_names,
                                                                 head_names,
                                                                 **kwargs
                                                                )
        summary_result = self.build_summary(removed_table, added_table, same_table)
        print(summary_result)
        self.write_output(summary_result,out)
        return summary_result

    def main(self, **kwargs):
        action = kwargs.pop('action')

        if action == 'pr':
            self.pr(
                    event_id=kwargs.pop('event_id'),
                    out=kwargs.pop('out'),
                    **kwargs
                    )
        else:
            getattr(self, action)(**kwargs)

if __name__ == '__main__':
    args = TestHarnessResultsSummary.parseArgs()
    TestHarnessResultsSummary(args.database).main(**vars(args))

