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
        self.client = None
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
        Attempts to load the authentication environment from the
        "RESULTS_READER_AUTH_FILE" variable if it is available
        """
        auth_file = os.environ.get('RESULTS_READER_AUTH_FILE')
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

    def _getTestsEntry(self, folder_name: str, test_name: str,
                       limit: int = 50, unique_event: bool = False,
                       filter: Optional[dict] = None) -> list[dict]:
        """
        Internal helper for getting the raw database entries for a specific test.

        Args:
            folder_name: The folder name for the test
            test_name: The test name for the test
        Keyword args:
            limit: Limit of entries to return
            unique_event: Whether or not to only return one test per event
            filter: Additional filters to pass to the query
        """
        assert isinstance(limit, int)
        assert isinstance(unique_event, bool)
        assert isinstance(filter, (dict, NoneType))

        find = {"event_cause": {"$ne": "pr"},
                "folder_name": {"$eq": folder_name},
                "test_name": {"$eq": test_name}}
        if filter is not None:
            find.update(filter)

        entries = self.db.tests.find(find, limit=limit, sort=self.mongo_sort_id)

        values = []
        unique_shas = set()
        for entry in entries:
            # Only get the most recent value for this sha (in case of invalidation)
            if unique_event:
                event_sha = entry['event_sha']
                if event_sha in unique_shas:
                    continue

                unique_shas.add(event_sha)

            values.append(entry)

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

    def _getResultsEntry(self, id: ObjectId) -> dict:
        """
        Internal helper for getting the raw JSON entry
        for a entry in the "results" collection given the ID

        This is separate so that we can mock it in unit tests
        """
        assert isinstance(id, (ObjectId, str))
        if isinstance(id, int):
            id = ObjectId(id)

        value = self.db.results.find_one({"_id": id})
        if value is None:
            raise KeyError(f'No {self.database}.results entry with _id={id}')
        return value

    def _getTestHarnessResult(self, id: ObjectId) -> TestHarnessResults:
        """
        Internal helper for getting the TestHarnessResults representation
        of a specific results entry in the database, with caching
        """
        assert isinstance(id, (ObjectId, str))
        if isinstance(id, int):
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
