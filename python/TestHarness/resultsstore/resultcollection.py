# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements the ResultCollection, storage for multiple test results."""

from collections import defaultdict
from typing import Callable, Iterable, Tuple

from bson.objectid import ObjectId
from pymongo import DESCENDING
from pymongo.database import Database

from TestHarness.resultsstore.storedresults import StoredResult, StoredTestResult
from TestHarness.resultsstore.testdatafilters import ALL_TEST_KEYS, TestDataFilter
from TestHarness.resultsstore.utils import TestName, results_test_iterator


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

    def get_all_tests(
        self, filters: Iterable[TestDataFilter]
    ) -> dict[TestName, list[StoredTestResult]]:
        """
        Get all test results.

        This can be particularly expensive! Best to do it on a
        per-test basis with get_tests() if possible.

        Arguments:
        ---------
        filters : Iterable[TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        # Whether or not all data was requested
        all_data = TestDataFilter.ALL in filters
        # Keys within the test data to get, if not all
        keys = [v.value for v in filters]

        # Tests stored separately that we need to load later
        separate_tests: list[Tuple[ObjectId, StoredTestResult]] = []
        # Storage for all tests
        tests: dict[TestName, list[StoredTestResult]] = defaultdict(list)

        # Load the full documents; this is generally quicker
        # than trying to load just the data we need
        result_it = iter(self.results)
        with self.get_database().results.find(
            {"_id": {"$in": self.result_ids}},
            {"tests": 1},
            sort=[("_id", DESCENDING)],
        ) as cursor:
            for doc in cursor:
                id = doc["_id"]
                del doc["_id"]

                result = next(result_it)
                assert result.id == id

                for test in results_test_iterator(doc):
                    value = test.value
                    name = test.name
                    if not all_data:
                        value = {k: value[k] for k in keys}
                    test_result = None
                    if isinstance(value, dict):
                        test_result = StoredTestResult(
                            test.value, name, result, filters
                        )
                    else:
                        test_result = StoredTestResult({}, name, result, filters)
                        separate_tests.append((value, test_result))
                    tests[name].append(test_result)

        # Pull the data from the tests stored separately if needed
        if separate_tests:
            # Sort based on ID so that we can do the same
            # when getting the data from the pipeline,
            # removing the need for us to sort here
            separate_tests = sorted(separate_tests, key=lambda x: x[0], reverse=True)

            # Setup the pipeline for getting the separate tests
            pipeline = [
                # Select only the needed tests
                {"$match": {"_id": {"$in": [v[0] for v in separate_tests]}}},
            ]
            # If not getting all data, filter on just the data we need
            if not all_data:
                pipeline += [
                    {
                        "$project": {
                            **{key: 1 for key in keys},
                        }
                    },
                ]
            # And sort on ID (which was the sort test_ids were passed)
            pipeline += [{"$sort": {"_id": -1}}]

            # Update each previously-empty StoredTestResult with
            # the real test data
            separate_test_it = iter(separate_tests)
            with self.get_database().tests.aggregate(pipeline) as cursor:
                for doc in cursor:
                    id, test = next(separate_test_it)
                    assert id == doc["_id"]  # should be sorted
                    test._data = doc

        return tests

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
