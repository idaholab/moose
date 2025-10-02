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

        return parser.parse_args()

    @staticmethod
    def diff_table(results: TestHarnessResults, base_names: set[TestName],
                   head_names: set[TestName]) -> Tuple[Optional[list], Optional[list]]:
        """
        Compare test names between the base and current head, and return
        the difference

        Parameters
        ----------
        results : TestHarnessResults
            An object that provides access to test results for the head
        base_names : set of TestName
            The set of test names from the base commit or version.
        head_names : set of TestName
            The set of test names from the current head commit or version.

        Returns
        -------
        removed_table : list or None
            A list of test names that were present in the base but not in the head.
        added_table : list or None
            A list of newly added test names along with their runtime.

            Returns `None` if no new tests were removed or added.
        """
        assert isinstance(results, TestHarnessResults)
        assert isinstance(head_names,(set, NoneType))
        assert isinstance(base_names,(set, NoneType))

        removed_names = base_names - head_names
        if removed_names:
            removed_table = list(removed_names)
        else:
            removed_table = None

        add_names = head_names - base_names
        if add_names:
            added_table = []
            for test_name in add_names:
                test_result = results.get_test(test_name.folder, test_name.name)
                added_table.append([str(test_name), test_result.run_time])
        else:
            added_table = None
        return removed_table, added_table

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
        results : TestHarnessResults
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
        event_id = kwargs['event_id']
        assert isinstance(event_id, int)

        results = self.reader.getEventResults(event_id)
        if results is None:
            raise SystemExit(f'ERROR: Results do not exist for event {event_id}')
        test_names = set(results.test_names)
        base_sha = results.base_sha
        assert isinstance(base_sha, str)

        base_results = self.reader.getCommitResults(base_sha)
        if not isinstance(base_results, TestHarnessResults):
            print(f"\nComparison not available: no baseline results found for base SHA {base_sha}")
            return results, test_names, None
        base_test_names = set(base_results.test_names)
        return results, test_names, base_test_names

    def build_summary(self, removed_table: list, added_table: list) -> str:
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

        Returns
        -------
        summary : str
            A formatted string summarizing removed and new tests using GitHub-style format
        """
        assert isinstance(removed_table,(list,NoneType))
        assert isinstance(added_table,(list,NoneType))

        summary = []
        summary.append("### Removed Tests:")
        if removed_table:
            table = [[str(test_name)] for test_name in removed_table]
            summary.append(tabulate(table, headers=["Test Name"], tablefmt="github"))
        else:
            summary.append("#### No Removed Tests")

        summary.append("### New Tests:")
        if added_table:
            summary.append(tabulate(added_table, headers=["Test Name", "Run Time"], tablefmt="github"))
        else:
            summary.append("#### No New Tests")
        return "\n".join(summary)

    def pr(self, **kwargs) -> str:
        results, head_names, base_names = self.pr_test_names(**kwargs)
        assert isinstance(head_names,set)
        assert isinstance(base_names,(set,NoneType))
        if base_names is None:
            return
        removed_table, added_table = self.diff_table(results, base_names, head_names)
        print(self.build_summary(removed_table, added_table))

    def main(self, **kwargs):
        action = kwargs['action']
        getattr(self, action)(**kwargs)

if __name__ == '__main__':
    args = TestHarnessResultsSummary.parseArgs()
    TestHarnessResultsSummary(args.database).main(**vars(args))

