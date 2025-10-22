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
from typing import Optional, Union, Iterator

import pymongo
from pymongo.cursor import Cursor
from bson.objectid import ObjectId

from TestHarness.resultsstore.storedresults import StoredResult, StoredTestResult
from TestHarness.resultsstore.auth import Authentication, load_authentication, has_authentication

NoneType = type(None)

class ResultsReader:
    """
    Utility for reading test harness results stored in a mongodb database
    """
    # Default sort ID from mongo
    mongo_sort_id = sort=[('_id', pymongo.DESCENDING)]

    def __init__(self, database: str, client: Optional[pymongo.MongoClient | Authentication] = None):
        self._client: pymongo.MongoClient = None
        assert isinstance(database, str)

        # No client; test the environment
        if client is None:
            client = self.loadEnvironmentAuthentication()
            if client is None:
                raise ValueError("Must specify either 'client' or set RESULTS_READER_AUTH_FILE with credentials")
        if isinstance(client, pymongo.MongoClient):
            self._client = client
        elif isinstance(client, Authentication):
            self._client = pymongo.MongoClient(client.host, username=client.username, password=client.password)
        else:
            raise TypeError(f"Invalid type for 'client'")

        # Get the database
        if database not in self._client.list_database_names():
            raise ValueError(f'Database {database} not found')
        self._db = self._client.get_database(database)

        # Cached results, by ID
        self._results: dict[ObjectId, Optional[StoredResult]] = {}
        # Cached results by PR number
        self._pr_num_results: dict[int, Optional[StoredResult]] = {}
        # Cached results by event ID
        self._event_id_results: dict[int, Optional[StoredResult]] = {}
        # Cached results by commit
        self._event_sha_results: dict[str, Optional[StoredResult]] = {}

        # Cached StoredResult objects for push events, latest to oldest
        self._latest_push_results: list[StoredResult] = []
        # The last push event that we searched
        self._last_latest_push_event_id: Optional[ObjectId] = None

    def __del__(self):
        # Clean up the client if it is loaded
        if self._client is not None:
            self._client.close()

    @staticmethod
    def loadEnvironmentAuthentication() -> Optional[Authentication]:
        """
        Attempts to first load the authentication environment from
        env vars RESULTS_READER_AUTH_[HOST,USERNAME,PASSWORD] if
        available. Otherwise, tries to load the authentication
        environment from the file set by env var
        RESULTS_READER_AUTH_FILE if it is available.
        """
        return load_authentication('RESULTS_READER')

    @staticmethod
    def hasEnvironmentAuthentication() -> bool:
        """
        Checks whether or not environment authentication is available
        """
        return has_authentication('RESULTS_READER')

    def getTestResults(self, folder_name: str, test_name: str, limit: int = 50,
                       pr_num: Optional[int] = None) -> list[StoredTestResult]:
        """
        Get the StoredTestResults given a specific test.

        Args:
            folder_name: The folder name for the test
            test_name: The test name for the test
        Optional args:
            limit: The limit in the number of results to get
            pr_num: A pull request to also pull from
        """
        test_results: list[StoredTestResult] = []

        # Append the PR result at the top, if any
        if pr_num is not None:
            pr_result = self.getPRResults(pr_num)
            if pr_result is not None:
                if pr_result.has_test(folder_name, test_name):
                    test_results.append(pr_result.get_test(folder_name, test_name))

        # Get the event results, limited to limit or
        # (limit - 1) if we have a PR
        num = limit - len(test_results)
        for result in self.iterateLatestPushResults(num):
            if result.has_test(folder_name, test_name):
                test_result = result.get_test(folder_name, test_name)
                test_results.append(test_result)
            if len(test_results) == limit:
                break

        return test_results

    def _findResults(self, *args, **kwargs) -> list[dict]:
        """
        Helper for querying the results database collection

        Used so that it can be easily movcked in unit tests
        """
        kwargs['sort'] = self.mongo_sort_id
        with self._db.results.find(*args, **kwargs) as cursor:
           return [d for d in cursor]

    def getLatestPushResults(self, num: int) -> list[StoredResult]:
        """
        Get the latest results from push events, newest to oldest
        """
        assert isinstance(num, int)
        assert num > 0

        results = []
        for result in self.iterateLatestPushResults(num=num):
            results.append(result)
            if len(results) == num:
                break
        return results

    def iterateLatestPushResults(self, num: Optional[int] = None) -> Iterator[StoredResult]:
        """
        Iterate through the latest unique push event results

        The number of needed results can optionally be passed
        in order to optimize the number of database queries
        """
        assert isinstance(num, (int, NoneType))

        # If we were given a number we want, pull exactly
        # however many more we need the first time around
        if isinstance(num, int) and num > 0:
            batch_size = num - len(self._latest_push_results)
        # Otherwise, just pull one at a time
        else:
            batch_size = 1

        i = 0

        while True:
            # At end of cache, pull more results
            if i == len(self._latest_push_results):
                # Searching for only events
                filter = {"event_cause": {"$ne": "pr"}}
                # Only search past what we've searched so far
                if self._last_latest_push_event_id is not None:
                    filter['_id'] = {"$lt": self._last_latest_push_event_id}

                cursor = self._findResults(filter, limit=batch_size)

                # After the first batch size (could be set by num),
                # only pull one at a time. We should only ever
                # hit this case when num is set if we find multiple
                # entries for the same event (due to invalidation)
                if isinstance(num, int):
                    batch_size = 1

                for data in cursor:
                    id = data.get('_id')
                    assert isinstance(id, ObjectId)

                    # So that we know where to search next time
                    self._last_latest_push_event_id = id

                    # Due to invalidation, we could have multiple results
                    # from the same event. If we already have this event
                    # (the latest version of it), skip this one
                    event_id = data.get('event_id')
                    if event_id is not None and \
                        any(r.event_id == event_id for r in self._latest_push_results):
                        continue

                    # Build/get the result and store it for future use
                    result = self._buildResults(data)
                    self._latest_push_results.append(result)

            if i < len(self._latest_push_results):
                yield self._latest_push_results[i]
                i += 1
            else:
                return

    def _getCachedResults(self, index: str, value) -> StoredResult:
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
        data = self._db.results.find_one(filter, sort=self.mongo_sort_id)

        # No such thing
        if data is None:
            cache[value] = None
            return None

        # Has an entry, so build it
        result = self._buildResults(data)
        cache[value] = result
        return result

    def getEventResults(self, event_id: int) -> Union[StoredResult, None]:
        """
        Get the StoredResult for a given event, if any
        """
        assert isinstance(event_id, int)
        return self._getCachedResults('event_id', event_id)

    def getPRResults(self, pr_num: int) -> Union[StoredResult, None]:
        """
        Get the StoredResult for a given PR, if any
        """
        assert isinstance(pr_num, int)
        return self._getCachedResults('pr_num', pr_num)

    def getCommitResults(self, commit_sha: str) -> Union[StoredResult, None]:
        """
        Get the StoredResult for a given commit, if any
        """
        assert isinstance(commit_sha, str)
        assert len(commit_sha) == 40
        return self._getCachedResults('event_sha', commit_sha)

    def _buildResults(self, data: dict) -> StoredTestResult:
        """
        Internal helper for building a StoredTestResult given
        the data from a results entry, also storing it in the cache
        """
        assert isinstance(data, dict)
        assert '_id' in data

        id = data['_id']
        assert isinstance(id, ObjectId)

        result = self._results.get(id)
        if result is None:
            try:
                result = StoredResult(data, self._db)
            except Exception as e:
                raise ValueError(f'Failed to build result _id={id}') from e
            self._results[id] = result

        return result
