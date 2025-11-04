# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements ResultsReader for reading results from a database."""

from dataclasses import dataclass
from typing import Iterator, Optional

import pymongo
from bson.objectid import ObjectId
from pymongo.database import Database

from TestHarness.resultsstore.auth import (
    Authentication,
    has_authentication,
    load_authentication,
)
from TestHarness.resultsstore.storedresults import StoredResult, StoredTestResult
from TestHarness.resultsstore.utils import TestName

NoneType = type(None)


@dataclass(frozen=True)
class ResultsReaderContextManager:
    """Storage for the ResultsReader context manager."""

    reader: "ResultsReader"


class ResultsReader:
    """Utility for reading test harness results stored in a mongodb database."""

    # Default sort ID from mongo
    mongo_sort_id = sort = [("_id", pymongo.DESCENDING)]

    def __init__(
        self,
        database_name: str,
        client: Optional[pymongo.MongoClient | Authentication] = None,
        check: bool = True,
        timeout: float = 5.0,
    ):
        """
        Initialize the reader.

        Parameters
        ----------
        database_name : str
            The name of the database to connect to.

        Optional parameters
        -------------------
        client : Optional[pymongo.MongoClient | Authentication]
            The client to use or authentication to connect with.
        check : bool
            Whether or not to validate result data types (default = True).
        timeout : Number
            Timeout time in seconds for interacting with the database.

        """
        assert isinstance(database_name, str)

        # The name of the database
        self._database_name: str = database_name
        # Whether or not to validate result data types on build
        self._check = check
        # Timeout time in seconds for database calls
        self._timeout = float(timeout)

        # The mongo client, possibly setup on use
        self._client: Optional[pymongo.MongoClient] = None
        # The mongo database, setup first on use
        self._database: Optional[Database] = None
        # The authentication for when we don't have a client
        self._authentication: Optional[Authentication] = None

        # No client, load from the environment
        if client is None:
            auth = self.loadEnvironmentAuthentication()
            if auth is None:
                raise ValueError(
                    "Must specify either 'client' or set RESULTS_READER_AUTH_FILE "
                    "with credentials"
                )
            self._authentication = auth
        # Passed an authentication, use it intead
        elif isinstance(client, Authentication):
            self._authentication = client
        # Passed a client directly
        elif isinstance(client, pymongo.MongoClient):
            self._client = client
        # Unknown type
        else:
            raise TypeError("Invalid type for 'client'")

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
        """Clean up the client if it is loaded."""
        self.close()

    def __enter__(self) -> ResultsReaderContextManager:
        """
        Enter a context manager for the ResultsReader.

        Will cleanup the client on exit.
        """
        return ResultsReaderContextManager(reader=self)

    def __exit__(self, exc_type, exc_value, exc_traceback):
        """
        Exit for the context manager.

        Will cleanup the client if it exists.
        """
        self.close()

    @staticmethod
    def loadEnvironmentAuthentication() -> Optional[Authentication]:
        """Attempt to load the authentication environment."""
        return load_authentication("RESULTS_READER")

    @staticmethod
    def hasEnvironmentAuthentication() -> bool:
        """Check whether or not environment authentication is available."""
        return has_authentication("RESULTS_READER")

    @property
    def check(self) -> bool:
        """Whether or not to validate data types on build."""
        return self._check

    @property
    def client(self) -> pymongo.MongoClient:
        """
        Get the pymongo client.

        On first call this will setup the client if not already.
        """
        if self._client is None:
            auth = self._authentication
            assert isinstance(auth, Authentication)
            self._client = pymongo.MongoClient(
                auth.host,
                username=auth.username,
                password=auth.password,
                timeoutMS=self._timeout * 1000,
            )
        assert isinstance(self._client, pymongo.MongoClient)
        return self._client

    def _databaseGetter(self) -> Database:
        """
        Get the pymongo database.

        On the first call this will query the database from the client.

        Passed to constructed StoredResult objects so that
        they can also setup the database on first use if needed.
        """
        if self._database is None:
            name = self._database_name
            client = self.client
            if name not in client.list_database_names():
                raise ValueError(f"Database {name} not found")
            self._database = client.get_database(name)
        assert isinstance(self._database, Database)
        return self._database

    @property
    def database(self) -> Database:
        """
        Get the pymongo database.

        On first call this will query the database from the client.
        """
        return self._databaseGetter()

    def close(self):
        """Close the database connection if it exists."""
        if self._client is not None:
            self._client.close()

    def getTestResults(
        self,
        name: TestName,
        limit: int = 50,
        pr_num: Optional[int] = None,
    ) -> list[StoredTestResult]:
        """
        Get the StoredTestResults given a specific test.

        Args:
            name: The combined name for the test
        Optional args:
            limit: The limit in the number of results to get
            pr_num: A pull request to also pull from

        """
        test_results: list[StoredTestResult] = []

        # Append the PR result at the top, if any
        if pr_num is not None:
            pr_result = self.getPRResults(pr_num)
            if (
                pr_result is not None
                and (test := pr_result.query_test(name)) is not None
            ):
                test_results.append(test)

        # Get the event results, limited to limit or
        # (limit - 1) if we have a PR
        num = limit - len(test_results)
        for result in self.iterateLatestPushResults(num):
            if (test := result.query_test(name)) is not None:
                test_results.append(test)
            if len(test_results) == limit:
                break

        return test_results

    def _findResults(self, *args, **kwargs) -> list[dict]:
        """
        Query the results database collection.

        Used so that it can be easily mocked in unit tests.
        """
        kwargs["sort"] = self.mongo_sort_id
        with self.database.results.find(*args, **kwargs) as cursor:
            return [d for d in cursor]

    def getLatestPushResults(self, num: int) -> list[StoredResult]:
        """Get the latest results from push events, newest to oldest."""
        assert isinstance(num, int)
        assert num > 0

        results = []
        for result in self.iterateLatestPushResults(num=num):
            results.append(result)
            if len(results) == num:
                break
        return results

    def iterateLatestPushResults(
        self, num: Optional[int] = None
    ) -> Iterator[StoredResult]:
        """
        Iterate through the latest unique push event results.

        The number of needed results can optionally be passed
        in order to optimize the number of database queries.
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
                    filter["_id"] = {"$lt": self._last_latest_push_event_id}

                cursor = self._findResults(filter, limit=batch_size)

                # After the first batch size (could be set by num),
                # only pull one at a time. We should only ever
                # hit this case when num is set if we find multiple
                # entries for the same event (due to invalidation)
                if isinstance(num, int):
                    batch_size = 1

                for data in cursor:
                    id = data.get("_id")
                    assert isinstance(id, ObjectId)

                    # So that we know where to search next time
                    self._last_latest_push_event_id = id

                    # Due to invalidation, we could have multiple results
                    # from the same event. If we already have this event
                    # (the latest version of it), skip this one
                    event_id = data.get("event_id")
                    if event_id is not None and any(
                        r.event_id == event_id for r in self._latest_push_results
                    ):
                        continue

                    # Build/get the result and store it for future use
                    result = self._buildResults(data)
                    self._latest_push_results.append(result)

            if i < len(self._latest_push_results):
                yield self._latest_push_results[i]
                i += 1
            else:
                return

    def _getCachedResults(self, index: str, value) -> Optional[StoredResult]:
        """Get a result given a filter and store it in the cache."""
        cache = getattr(self, f"_{index}_results")

        # Value exists in the cache
        cached_value = cache.get(value)
        if cached_value is not None:
            return cached_value

        # Search for the value
        filter = {index: {"$eq": value}}
        data = self.database.results.find_one(filter, sort=self.mongo_sort_id)

        # No such thing
        if data is None:
            cache[value] = None
            return None

        # Has an entry, so build it
        result = self._buildResults(data)
        cache[value] = result
        return result

    def getEventResults(self, event_id: int) -> Optional[StoredResult]:
        """Get the StoredResult for a given event, if any."""
        assert isinstance(event_id, int)
        return self._getCachedResults("event_id", event_id)

    def getPRResults(self, pr_num: int) -> Optional[StoredResult]:
        """Get the StoredResult for a given PR, if any."""
        assert isinstance(pr_num, int)
        return self._getCachedResults("pr_num", pr_num)

    def getCommitResults(self, commit_sha: str) -> Optional[StoredResult]:
        """Get the StoredResult for a given commit, if any."""
        assert isinstance(commit_sha, str)
        assert len(commit_sha) == 40
        return self._getCachedResults("event_sha", commit_sha)

    def _buildResults(self, data: dict) -> StoredResult:
        """Build a StoredTestResult given its data and store in the cache."""
        assert isinstance(data, dict)
        assert "_id" in data

        id = data["_id"]
        assert isinstance(id, ObjectId)

        result = self._results.get(id)
        if result is None:
            try:
                result = StoredResult(
                    data, database_getter=self._databaseGetter, check=self.check
                )
            except Exception as e:
                raise ValueError(f"Failed to build result _id={id}") from e
            self._results[id] = result

        return result
