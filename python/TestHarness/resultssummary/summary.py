#!/usr/bin/env python3
import argparse
from typing import Tuple

from TestHarness.resultsreader.reader import TestHarnessResultsReader
from TestHarness.resultsreader.results import TestHarnessResults, TestName
from tabulate import tabulate

class TestHarnessResultsSummary:
    def __init__(self, database: str):
        self.reader = TestHarnessResultsReader(database)

    @staticmethod
    def parseArgs() -> argparse.Namespace:
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
                   head_names: set[TestName]) -> Tuple[list, list]:
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
        event_id = kwargs['event_id']
        assert isinstance(event_id, int)

        results = self.reader.getEventResults(kwargs['event_id'])
        test_names = set(results.test_names)
        
        base_sha = results.base_sha
        base_results = self.reader.getCommitResults(base_sha)
        base_test_names = set(base_results.test_names)

        return results, test_names, base_test_names
    

    def build_summary(self, removed_table: list, added_table: list) -> str:
        summary = []

        summary.append("\nRemoved Tests:")
        if removed_table:
            table = [[str(test_name)] for test_name in removed_table]
            summary.append(tabulate(table, headers=["Test Name"], tablefmt="github"))
        else:
            summary.append("No Removed Tests")

        summary.append("\nNew Tests:")
        if added_table:
            summary.append(tabulate(added_table, headers=["Test Name", "Run Time"], tablefmt="github"))
        else:
            summary.append("No New Tests")

        return "\n".join(summary)


    def pr(self, **kwargs) -> str:
        results, head_names, base_names = self.pr_test_names(**kwargs)
        removed_table, added_table = self.diff_table(results, base_names, head_names)
        print(self.build_summary(removed_table, added_table))

    def main(self, **kwargs):
        action = kwargs['action']
        getattr(self, action)(**kwargs)
        
if __name__ == '__main__':
    args = TestHarnessResultsSummary.parseArgs()
    TestHarnessResultsSummary(args.database).main(**vars(args))

