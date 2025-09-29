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
from typing import Optional, Union, Generator

import pymongo
from pymongo.cursor import Cursor
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

        # Cached results, by ID
        self._results: dict[ObjectId, TestHarnessResults | None] = {}
        # Cached results by PR number
        self._pr_num_results: dict[int, TestHarnessResults] = {}
        # Cached results by event ID
        self._event_id_results: dict[int, TestHarnessResults] = {}
        # Cached results by commit
        self._event_sha_results: dict[str, TestHarnessResults] = {}

        # Cached TestHarnessResults objects for events, in order from latest
        self._latest_event_results: list[TestHarnessResults] = []
        # The last event that we searched
        self._last_latest_event_id: ObjectId | None = None

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
            all_auth_vars = ' '.join(map(var_name, all_auth_keys))
            raise ValueError(f'All environment variables "{all_auth_vars}" must be set for authentication')

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

    def getTestResults(self, folder_name: str, test_name: str, limit: int = 50,
                       pr_num: Optional[int] = None) -> list[TestHarnessTestResult]:
        """
        Get the TestHarnessTestResults given a specific test.

        Args:
            folder_name: The folder name for the test
            test_name: The test name for the test
        Optional args:
            limit: The limit in the number of results to get
            pr_num: A pull request to also pull from
        """
        test_results: list[TestHarnessTestResult] = []

        # Append the PR result at the top, if any
        if pr_num is not None:
            pr_result = self.getPRResults(pr_num)
            if pr_result is not None:
                test_results.append(pr_result)

        # Get the event results
        for result in self.getLatestEventResults():
            if result.has_test(folder_name, test_name):
                test_result = result.get_test(folder_name, test_name)
                test_results.append(test_result)
            if len(test_results) == limit:
                break

        return test_results

    def _findResults(self, *args, **kwargs) -> Cursor:
        """
        Helper for querying the results database collection

        Used so that it can be easily movcked in unit tests
        """
        return self.db.results.find(*args, **kwargs, sort=self.mongo_sort_id)

    def getLatestEventResults(self) -> Generator[TestHarnessResults]:
        """
        Iterate through the latest unique event results
        """
        i = 0
        while True:
            # At end of cache, pull more results
            if i == len(self._latest_event_results):
                # Searching for only events
                filter = {"event_cause": {"$ne": "pr"}}
                # Only search past what we've searched so far
                if self._last_latest_event_id is not None:
                    filter['_id'] = {"$lt": self._last_latest_event_id}

                cursor = self._findResults(filter, limit=10)

                for data in cursor:
                    id = data.get('_id')
                    assert isinstance(id, ObjectId)

                    # So that we know where to search next time
                    self._last_latest_event_id = id

                    # Due to invalidation, we could have multiple results
                    # from the same event. If we already have this event
                    # (the latest version of it), skip this one
                    event_id = data.get('event_id')
                    if event_id is not None and \
                        [r for r in self._latest_event_results if r.event_id == event_id]:
                        continue

                    # Build/get the result and store it for future use
                    result = self._buildResults(data)
                    self._latest_event_results.append(result)

            if i != len(self._latest_event_results):
                yield self._latest_event_results[i]
                i += 1
            else:
                return

    def _getCachedResults(self, index: str, value) -> TestHarnessResults:
        """
        Internal helper for getting a result given a filter and storing
        it in a cache given a key
        """
        cache = getattr(self, f'_{index}_results')

        # Value exists in the cache
        cached_value = cache.get(value)
        if cached_value is not None:
            return cached_value

        # Search for the value
        filter = {index: {"$eq": value}}
        data = self.db.results.find_one(filter, sort=self.mongo_sort_id)

        # No such thing
        if data is None:
            cache[value] = None
            return None

        # Has an entry, so build it
        result = self._buildResults(data)
        cache[value] = result
        return result

    def getEventResults(self, event_id: int) -> Union[TestHarnessResults, None]:
        """
        Get the TestHarnessResults for a given event, if any
        """
        assert isinstance(event_id, int)
        return self._getCachedResults('event_id', event_id)

    def getPRResults(self, pr_num: int) -> Union[TestHarnessResults, None]:
        """
        Get the TestHarnessResults for a given PR, if any
        """
        assert isinstance(pr_num, int)
        return self._getCachedResults('pr_num', pr_num)

    def getCommitResults(self, commit_sha: str) -> Union[TestHarnessResults, None]:
        """
        Get the TestHarnessResults for a given commit, if any
        """
        assert isinstance(commit_sha, str)
        assert len(commit_sha) == 40
        return self._getCachedResults('event_sha', commit_sha)

    def _buildResults(self, data: dict) -> TestHarnessTestResult:
        """
        Internal helper for building a TestHarnessTestResult given
        the data from a results entry, also storing it in the cache
        """
        assert isinstance(data, dict)
        assert '_id' in data

        id = data['_id']
        assert isinstance(id, ObjectId)

        result = self._results.get(id)
        if result is None:
            try:
                result = TestHarnessResults(data, self.db)
            except Exception as e:
                raise Exception(f'Failed to build result _id={id}') from e
            self._results[id] = result

        return result
