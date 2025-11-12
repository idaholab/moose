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
from TestHarness.resultsstore.resultcollection import ResultCollection
from TestHarness.resultsstore.storedresult import StoredResult

NoneType = type(None)


@dataclass(frozen=True)
class ResultsReaderContextManager:
    """Storage for the ResultsReader context manager."""

    reader: "ResultsReader"


class ResultsReader:
    """Utility for reading test harness results stored in a mongodb database."""

    # Default sort ID from mongo
    mongo_sort_id = [("_id", pymongo.DESCENDING)]

    def __init__(
        self,
        database_name: str,
        client: Optional[pymongo.MongoClient] = None,
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
        client : Optional[pymongo.MongoClient]
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

        # The mongo client, setup on first use
        self._client: Optional[pymongo.MongoClient] = None
        # The mongo database, setup on first use
        self._database: Optional[Database] = None
        # The authentication for when we don't have a client
        self._authentication: Optional[Authentication] = None

        # No client, load from the environment
        if client is None:
            auth = self.load_authentication()
            if auth is None:
                raise ValueError(
                    "Must specify either 'client' or set RESULTS_READER_AUTH_FILE "
                    "with credentials"
                )
            self._authentication = auth
        # Passed an authentication, use it intead
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
        # The final push event that was found
        self._found_final_push_event: bool = False

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
    def load_authentication() -> Optional[Authentication]:
        """Attempt to load the authentication environment."""
        return load_authentication("RESULTS_READER")

    @staticmethod
    def has_authentication() -> bool:
        """Check whether or not environment authentication is available."""
        return has_authentication("RESULTS_READER")

    @property
    def check(self) -> bool:
        """Whether or not to validate data types on build."""
        return self._check

    def get_client(self) -> pymongo.MongoClient:
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
        return self._client

    def get_database(self) -> Database:
        """
        Get the pymongo database.

        On first call this will query the database from the client
        and start the connection.
        """
        if self._database is None:
            self._database = self.get_client().get_database(self._database_name)
        assert isinstance(self._database, Database)
        return self._database

    def close(self):
        """Close the database connection if it exists."""
        if self._client is not None:
            self._client.close()

    def _find_results(self, filter: dict, limit: Optional[int]) -> list[dict]:
        """
        Perform a find in the results collection, without the tests key.

        Does not get data in the "tests" key.
        """
        assert isinstance(filter, dict)
        assert isinstance(limit, (type(None), int))

        kwargs = {}
        if limit is not None:
            kwargs["limit"] = limit

        with self.get_database().results.find(
            filter, {"tests": 0}, sort=self.mongo_sort_id, **kwargs
        ) as cursor:
            return [d for d in cursor]

    def _aggregate_results(self, pipeline: list[dict]) -> list[dict]:
        """
        Perform an aggregation in the results collection.

        Does not get data in the "tests" key.
        """
        assert isinstance(pipeline, list)

        with self.get_database().results.aggregate(pipeline) as cursor:
            return [d for d in cursor]

    def _build_result(self, data: dict) -> StoredResult:
        """Build a result given its data and store in the cache."""
        assert isinstance(data, dict)

        id = data.get("_id")
        assert isinstance(id, ObjectId)

        result = self._results.get(id)
        if result is None:
            try:
                result = StoredResult(
                    data,
                    check=self.check,
                )
            except Exception as e:
                raise ValueError(f"Failed to build result _id={id}") from e
            self._results[id] = result

        return result

    def latest_push_results_iterator(
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

        def get_pipeline(batch_size):
            return [
                # Older than the last ID (if any) and only on push
                {
                    "$match": {
                        **(
                            {"_id": {"$lt": self._last_latest_push_event_id}}
                            if self._last_latest_push_event_id is not None
                            else {}
                        ),
                        "event_cause": {"$ne": "pr"},
                    }
                },
                # Separate results with and without event ID
                {
                    "$facet": {
                        # Only allow one (latest) result per event ID
                        "with_event_id": [
                            {"$match": {"event_id": {"$exists": True}}},
                            {"$sort": {"_id": -1}},
                            {
                                "$group": {
                                    "_id": "$event_id",
                                    "doc": {"$first": "$$ROOT"},
                                }
                            },
                            {"$replaceRoot": {"newRoot": "$doc"}},
                        ],
                        "no_event_id": [
                            {"$match": {"event_id": {"$exists": False}}},
                            {"$sort": {"_id": -1}},
                        ],
                    },
                },
                # Combine results with and without ID
                {
                    "$project": {
                        "docs": {"$concatArrays": ["$with_event_id", "$no_event_id"]}
                    }
                },
                {"$unwind": "$docs"},
                {"$replaceRoot": {"newRoot": "$docs"}},
                # Final sort on latest
                {"$sort": {"_id": -1}},
                # Limit to size
                {"$limit": batch_size},
                # Ignore tests entry
                {"$project": {"tests": 0}},
            ]

        while True:
            # At end of cache, pull more results
            if i == len(self._latest_push_results) and not self._found_final_push_event:
                docs = self._aggregate_results(get_pipeline(batch_size))

                # After the first batch size (could be set by num),
                # only pull one at a time. We should only ever
                # hit this case when num is set if we find multiple
                # entries for the same event (due to invalidation)
                if isinstance(num, int):
                    batch_size = 1

                built_results = [self._build_result(doc) for doc in docs]
                self._latest_push_results += built_results
                if built_results:
                    self._last_latest_push_event_id = built_results[-1].id

            # Have a shared result to yield
            if i < len(self._latest_push_results):
                result = self._latest_push_results[i]

                # Make sure IDs are decreasing
                if i > 1:
                    assert result.id < self._latest_push_results[i - 1].id

                yield result
                i += 1
            # No more results, and we didn't find any more
            else:
                self._found_final_push_event = True
                return

    def get_latest_push_results(self, num: int) -> ResultCollection:
        """Get the latest results from push events, newest to oldest."""
        assert isinstance(num, int)
        assert num > 0

        results = []
        for result in self.latest_push_results_iterator(num=num):
            results.append(result)
            if len(results) == num:
                break

        return ResultCollection(results, self.get_database)

    def get_cached_result(self, index: str, value) -> Optional[ResultCollection]:
        """Get a result given a filter and store it in the cache."""
        cache = getattr(self, f"_{index}_results")

        # Value exists in the cache
        cached_value = cache.get(value)
        if cached_value is not None:
            return ResultCollection([cached_value], self.get_database)

        # Search for the value
        docs = self._find_results({index: {"$eq": value}}, limit=1)

        # No such thing
        if not docs:
            cache[value] = None
            return None

        # Has an entry, so build it
        assert len(docs) == 1
        data = docs[0]

        # Should be the proper event
        assert data[index] == value

        # Build it and return
        result = self._build_result(data)
        cache[value] = result
        return ResultCollection([result], self.get_database)

    def get_event_result(self, event_id: int) -> Optional[ResultCollection]:
        """Get the latest result for the given event, if any."""
        assert isinstance(event_id, int)
        return self.get_cached_result("event_id", event_id)

    def get_pr_result(self, pr_num: int) -> Optional[ResultCollection]:
        """Get the latest result for the given PR, if any."""
        assert isinstance(pr_num, int)
        return self.get_cached_result("pr_num", pr_num)

    def get_commit_result(self, commit_sha: str) -> Optional[ResultCollection]:
        """Get the latest result for the given commit, if any."""
        assert isinstance(commit_sha, str)
        assert len(commit_sha) == 40
        return self.get_cached_result("event_sha", commit_sha)
