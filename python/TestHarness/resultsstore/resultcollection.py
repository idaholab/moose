# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements the ResultCollection, storage for multiple test results."""

from typing import Callable, Iterable

from bson.objectid import ObjectId
from pymongo.database import Database

from TestHarness.resultsstore.storedresults import StoredResult, StoredTestResult
from TestHarness.resultsstore.testdatafilters import ALL_TEST_KEYS, TestDataFilter
from TestHarness.resultsstore.utils import TestName


class ResultCollection:
    """
    Stores multiple test results and exposes methods to loading tests.

    These objects wrap the results returned by the ResultsReader.
    """

    def __init__(
        self,
        results: list[StoredResult],
        database_getter: Callable[[], Database],
    ):
        """
        Initialize state.

        Arguments:
        ---------
        results : list[StoredResult]
            The results in the collection.
        database_getter : Callable[[], Database]
            Function to be called to get the database.

        """
        assert isinstance(results, list)
        assert isinstance(database_getter, Callable)

        # The underlying results in the collection
        self._results = results

        # Method to call to get the database
        self._database_getter = database_getter

    @property
    def result_ids(self) -> list[ObjectId]:
        """Get the IDs of the results in the collection."""
        return [r.id for r in self.results]

    @property
    def results(self) -> list[StoredResult]:
        """Get the underlying results."""
        return self._results

    def get_database(self) -> Database:
        """Get the database."""
        return self._database_getter()

    def get_tests(
        self, name: TestName, filters: Iterable[TestDataFilter]
    ) -> list[StoredTestResult]:
        """
        Get the results for the test with the given name.

        Arguments:
        ---------
        name : TestName
            The name of the test.
        filters : Iterable[TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        # Path in the result doc for this test
        query_path = name.mongo_path.query_path

        # Keys within test data to get
        keys = (
            ALL_TEST_KEYS
            if TestDataFilter.ALL in filters
            else [v.value for v in filters]
        )

        pipeline = [
            # Filter on results in the collection
            {"$match": {"_id": {"$in": self.result_ids}}},
            # Join tests that aren't stored within the result
            {
                "$lookup": {
                    "from": "tests",
                    "localField": query_path,
                    "foreignField": "_id",
                    "as": "separate_test",
                    "pipeline": [
                        {
                            "$project": {
                                **{key: 1 for key in keys},
                            }
                        }
                    ],
                }
            },
            # Combine keys from separate and embedded tests
            {
                "$addFields": {
                    "value": {
                        "$ifNull": [{"$first": "$separate_test"}, f"${query_path}"]
                    }
                }
            },
            # Only get requested keys
            {
                "$project": {
                    "test_id": "$value._id",  # only for foreign tests
                    **{key: f"$value.{key}" for key in keys},
                }
            },
            # Sort on ID, for which our results are sorted
            {"$sort": {"_id": -1}},
        ]

        # Build test results from documents
        tests = []
        result_it = iter(self.results)
        with self.get_database().results.aggregate(pipeline) as cursor:
            for doc in cursor:
                id = doc["_id"]
                del doc["_id"]

                if test_id := doc.get("test_id"):
                    doc["_id"] = test_id

                result = next(result_it)
                assert result.id == id
                tests.append(StoredTestResult(doc, name, result, filters))

        return tests

    def get_test_names(self) -> set[TestName]:
        """Get all of the test names in the collection."""
        pipeline = [
            {"$match": {"_id": {"$in": self.result_ids}}},
            {
                "$project": {
                    "folder_tests": {
                        "$map": {
                            "input": {"$objectToArray": "$tests"},
                            "as": "f",
                            "in": {
                                "folder_name": "$$f.k",
                                "test_names": {
                                    "$map": {
                                        "input": {
                                            "$objectToArray": {
                                                "$ifNull": ["$$f.v.tests", {}]
                                            }
                                        },
                                        "as": "t",
                                        "in": "$$t.k",
                                    }
                                },
                            },
                        }
                    },
                },
            },
            {
                "$project": {
                    "tests": {
                        "$map": {
                            "input": {
                                "$zip": {
                                    "inputs": [
                                        "$folder_tests.folder_name",
                                        "$folder_tests.test_names",
                                    ]
                                }
                            },
                            "as": "pair",
                            "in": "$$pair",
                        }
                    },
                }
            },
        ]

        names = set()
        with self.get_database().results.aggregate(pipeline) as cursor:
            for doc in cursor:
                for folder_name, test_names in doc["tests"]:
                    names.update(
                        [TestName(folder_name, test_name) for test_name in test_names]
                    )

        return names
