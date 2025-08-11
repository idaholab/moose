#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import yaml
from dataclasses import dataclass
from typing import Optional

import pymongo
from bson.objectid import ObjectId

from TestHarness.resultsreader.results import TestHarnessResults, TestHarnessTestResult

NoneType = type(None)

class TestHarnessResultsReader:
    """
    Utility for reading test harness results stored in a mongodb database
    """
    # Default sort ID from mongo
    mongo_sort_id = sort=[('_id', pymongo.DESCENDING)]

    @dataclass
    class Authentication:
        """
        Helper class for storing the authentication to a mongo database
        """
        def __post_init__(self):
            assert isinstance(self.host, str)
            assert isinstance(self.username, str)
            assert isinstance(self.password, str)
            assert isinstance(self.port, (int, NoneType))

        # The host name
        host: str
        # The username
        username: str
        # The password
        password: str
        # The port
        port: int | None = None

    def __init__(self, database: str, client: Optional[pymongo.MongoClient | Authentication] = None):
        self.client: pymongo.MongoClient = None
        assert isinstance(database, str)

        # No client; test the environment
        if client is None:
            client = self.loadEnvironmentAuthentication()
            if client is None:
                raise ValueError("Must specify either 'client' or set RESULTS_READER_AUTH_FILE with credentials")
        if isinstance(client, pymongo.MongoClient):
            self.client = client
        elif isinstance(client, self.Authentication):
            self.client = pymongo.MongoClient(client.host, username=client.username, password=client.password)
        else:
            raise TypeError(f"Invalid type for 'client'")

        # Get the database
        self.database: str = database
        if database not in self.client.list_database_names():
            raise ValueError(f'Database {database} not found')
        self.db = self.client.get_database(database)

        # Cached TestHarness result objects, by ID
        self._test_harness_results: dict[ObjectId, TestHarnessResults | None] = {}

    def __del__(self):
        # Clean up the client if it is loaded
        if self.client is not None:
            self.client.close()

    @staticmethod
    def loadEnvironmentAuthentication() -> Authentication | None:
        """
        Attempts to first load the authentication environment from
        env vars RESULTS_READER_AUTH_[HOST,USERNAME,PASSWORD] if
        available. Otherwise, tries to load the authentication
        environment from the file set by env var
        RESULTS_READER_AUTH_FILE if it is available.
        """
        # Helpers for getting authentication variables
        var_name = lambda k: f'RESULTS_READER_AUTH_{k.upper()}'
        get_var = lambda k: os.environ.get(var_name(k))

        # Try to get authentication from env
        all_auth_keys = ['host', 'username', 'password']
        auth = {}
        for key in all_auth_keys:
            v = get_var(key)
            if v:
                auth[key] = v
        # Have all three set
        if len(auth) == 3:
            auth['port'] = get_var('port')
            return TestHarnessResultsReader.Authentication(**auth)
        # Have one or two but not all three set
        if len(auth) != 0:
            all_auth_vars = [var_name(k) for k in all_auth_keys]
            raise ValueError(f'All environment variables "{" ".join(all_auth_vars)}" must be set for authentication')

        # Try to get authentication from file
        auth_file = get_var('file')
        if auth_file is None:
            return None
        try:
            with open(auth_file, 'r') as f:
                values = yaml.safe_load(f)
            return TestHarnessResultsReader.Authentication(**values)
        except Exception as e:
            raise Exception(f"Failed to load credentials from '{auth_file}'") from e

    @staticmethod
    def hasEnvironmentAuthentication() -> bool:
        """
        Checks whether or not environment authentication is available
        """
        return TestHarnessResultsReader.loadEnvironmentAuthentication() is not None

    @staticmethod
    def _testFilter(folder_name: str, test_name: str) -> dict:
        """
        Helper for getting the mongo filter for a given test
        """
        return {"folder_name": {"$eq": folder_name}, "test_name": {"$eq": test_name}}

    def _getTestsPREntry(self, folder_name: str, test_name: str, pr_num: int,
                         filter: Optional[dict] = {}) -> dict | None:
        """
        Internal method for getting the pull request entry for a given test

        Args:
            folder_name: The folder name for the test
            test_name: The test name for the test
            pr_num: The pull request number
        Keyword args:
            filter: Additional filters to pass to the query
        """
        assert isinstance(folder_name, str)
        assert isinstance(test_name, str)
        assert isinstance(pr_num, int)

        find = {"pr_num": {"$eq": pr_num}}
        find.update(self._testFilter(folder_name, test_name))
        find.update(filter)

        entry = self.db.tests.find_one(find, sort=self.mongo_sort_id)
        if entry:
            return dict(entry)
        return None

    def _getTestsEntry(self, folder_name: str, test_name: str,
                       limit: int = 50, unique_event: bool = True,
                       filter: dict = {}, pr_num: Optional[int] = None) -> list[dict]:
        """
        Internal helper for getting the raw database entries for a specific test.

        Args:
            folder_name: The folder name for the test
            test_name: The test name for the test
        Keyword args:
            limit: Limit of entries to return
            unique_event: Whether or not to only return one test per event
            filter: Additional filters to pass to the query
            pr_num: Number of a PR to include in results if available
        """
        assert isinstance(limit, int)
        assert isinstance(unique_event, bool)
        assert isinstance(filter, (dict, NoneType))
        assert isinstance(pr_num, (int, NoneType))

        values = []

        # Find a single entry for this pull request, if requested and put at the top
        if pr_num is not None:
            pr_entry = self._getTestsPREntry(folder_name, test_name, pr_num, filter=filter)
            if pr_entry:
                values.append(pr_entry)

        # Filtering for a non-pr for these tests
        find_base = {"event_cause": {"$ne": "pr"}}
        find_base.update(self._testFilter(folder_name, test_name))
        find_base.update(filter)

        # Last ID to filter less than, if any
        last_id = None

        # Due to the fact that we could have multiple entries for one event
        # (with invalidation), we might need to do multiple queries to obtain
        # the number of entries requested
        unique_shas = set()
        # Be able to decrease the limit if we need to do extra loops
        find_event_limit = limit
        # Decrease how many we need to find if we found a PR
        if pr_num is not None and values:
            find_event_limit -= 1

        while True:
            filter_non_pr = {}
            if last_id is not None:
                filter_non_pr['_id'] = {'$lt': last_id}
            filter_non_pr.update(find_base)

            cursor = self.db.tests.find(filter_non_pr, limit=find_event_limit, sort=self.mongo_sort_id)

            entry_count = 0
            for entry in cursor:
                if len(values) == limit:
                    break

                last_id = entry['_id']
                entry_count += 1

                # Only get the most recent value for this sha (in case of invalidation)
                if unique_event:
                    event_sha = entry['event_sha']
                    if event_sha in unique_shas:
                        continue
                    unique_shas.add(event_sha)

                values.append(dict(entry))

            # Didn't find anything; done
            if not entry_count:
                break

            # If we're running again, decrease the limit to
            # however many we need left
            if len(values) < limit:
                find_event_limit = len(values) - limit

        return values

    def getTestResults(self, folder_name: str, test_name: str, **kwargs) -> list[TestHarnessTestResult]:
        """
        Get the TestHarnessTestResults given a specific test.

        Args:
            folder_name: The folder name for the test
            test_name: The test name for the test
        Keyword args:
            See _getTestsEntry() for additional arguments
        """
        entries = self._getTestsEntry(folder_name, test_name, **kwargs)

        values = []
        for entry in entries:
            result_id = entry['result_id']
            test_harness_results = self._getTestHarnessResult(result_id)
            test_result = TestHarnessTestResult(entry, test_harness_results)
            values.append(test_result)
        return values

    def _getResultsEntry(self, id: ObjectId | str) -> dict:
        """
        Internal helper for getting the raw JSON entry
        for a entry in the "results" collection given the ID

        This is separate so that we can mock it in unit tests
        """
        assert isinstance(id, (ObjectId, str))
        if isinstance(id, str):
            id = ObjectId(id)

        value = self.db.results.find_one({"_id": id})
        if value is None:
            raise KeyError(f'No {self.database}.results entry with _id={id}')
        return value

    def _getTestHarnessResult(self, id: ObjectId | str) -> TestHarnessResults:
        """
        Internal helper for getting the TestHarnessResults representation
        of a specific results entry in the database, with caching
        """
        assert isinstance(id, (ObjectId, str))
        if isinstance(id, str):
            id = ObjectId(id)

        # Cached this result already
        if id in self._test_harness_results:
            return self._test_harness_results[id]

        # Find with the given ID
        entry = self._getResultsEntry(id)

        # Build the representation
        result = TestHarnessResults(entry)

        # Store cached result
        self._test_harness_results[id] = result

        return result
