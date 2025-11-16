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
from typing import Callable, Iterable, Optional, Tuple, Union

from bson.objectid import ObjectId
from pymongo import DESCENDING
from pymongo.database import Database

from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.storedtestresult import StoredTestResult
from TestHarness.resultsstore.testdatafilters import (
    ALL_TEST_KEYS,
    TestDataFilter,
    filter_as_iterable,
    has_all_filter,
)
from TestHarness.resultsstore.utils import TestName, results_test_iterator


class ResultsCollectionBase:
    """
    Base for a collection of results.

    Enables having a collection that contains multiple
    results and a collection that has a single result,
    with the same underlying getters.
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
        assert len(results) > 0
        assert all(isinstance(v, StoredResult) for v in results)
        assert isinstance(database_getter, Callable)

        # The underlying results in the collection
        self._results = results

        # Method to call to get the database
        self._database_getter = database_getter

    @property
    def result_ids(self) -> list[ObjectId]:
        """Get the IDs of the results in the collection."""
        return [r.id for r in self._results]

    def get_database(self) -> Database:
        """Get the database."""
        return self._database_getter()

    def _get_all_tests(
        self, test_filter: Union[Iterable[TestDataFilter], TestDataFilter]
    ) -> dict[TestName, list[StoredTestResult]]:
        """
        Get all test results.

        Used in derived classes.

        Arguments:
        ---------
        test_filter : Union[Iterable[TestDataFilter], TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        # Possibly convert single value to iterable
        test_filter = filter_as_iterable(test_filter)

        # Keys to parse
        keys = (
            ALL_TEST_KEYS
            if has_all_filter(test_filter)
            else [v.value for v in test_filter]
        )

        # Tests stored separately that we need to load later
        separate_tests: list[Tuple[ObjectId, StoredTestResult]] = []
        # Storage for all tests
        tests: dict[TestName, list[StoredTestResult]] = defaultdict(list)

        # Load the full documents; this is generally quicker
        # than trying to load just the data we need
        result_it = iter(self._results)
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
                    test_result = None
                    if isinstance(value, dict):
                        value = {k: value[k] for k in keys if k in value}
                        test_result = StoredTestResult(value, name, result, test_filter)
                    else:
                        test_result = StoredTestResult({}, name, result, test_filter)
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
                # And just the data we need
                {
                    "$project": {
                        **{key: 1 for key in keys},
                    }
                },
                # And sort on ID (which was the sort test IDs were passed)
                {"$sort": {"_id": -1}},
            ]

            # Update each previously-empty StoredTestResult with
            # the real test data
            separate_test_it = iter(separate_tests)
            with self.get_database().tests.aggregate(pipeline) as cursor:
                for doc in cursor:
                    id, test = next(separate_test_it)
                    assert id == doc["_id"]  # should be sorted
                    test._data = doc

        return tests

    @staticmethod
    def _get_tests_pipeline(
        result_ids: list[ObjectId],
        name: TestName,
        test_filter: Iterable[TestDataFilter],
    ) -> list[dict]:
        # Path in the result doc for this test
        query_path = name.mongo_path.query_path

        keys = (
            ALL_TEST_KEYS
            if has_all_filter(test_filter)
            else [v.value for v in test_filter]
        )

        return [
            # Filter on results in the collection
            {"$match": {"_id": {"$in": result_ids}}},
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

    def _get_tests(
        self,
        name: TestName,
        test_filter: Union[Iterable[TestDataFilter], TestDataFilter],
    ) -> list[StoredTestResult]:
        """
        Get the results for the test with the given name.

        Used in derived classes.

        Arguments:
        ---------
        name : TestName
            The name of the test.
        test_filter : Union[Iterable[TestDataFilter], TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        # Possibly convert single value to iterable
        test_filter = filter_as_iterable(test_filter)

        result_it = iter(self._results)

        def build_test(doc: dict) -> StoredTestResult:
            # For looping through the results in order
            nonlocal result_it

            # _id here is the result ID, not the test ID
            id = doc["_id"]
            del doc["_id"]

            # And test_id here is the test ID, so replace
            if test_id := doc.get("test_id"):
                doc["_id"] = test_id
                del doc["test_id"]

            # Get the next result; we should be receiving these
            # in order
            result = next(result_it)
            assert result.id == id

            return StoredTestResult(doc, name, result, test_filter)

        # Build test results from documents
        pipeline = self._get_tests_pipeline(self.result_ids, name, test_filter)
        with self.get_database().results.aggregate(pipeline) as cursor:
            return [build_test(doc) for doc in cursor]

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


class ResultCollection(ResultsCollectionBase):
    """Store a single test result and exposes methods to loading tests."""

    def __init__(
        self,
        result: StoredResult,
        database_getter: Callable[[], Database],
    ):
        """
        Initialize state.

        Arguments:
        ---------
        result : StoredResult
            The result in the collection.
        database_getter : Callable[[], Database]
            Function to be called to get the database.

        """
        super().__init__([result], database_getter)

    @property
    def result(self) -> StoredResult:
        """Get the underlying result."""
        assert len(self._results) == 1
        return self._results[0]

    def get_all_tests(
        self, test_filter: Union[Iterable[TestDataFilter], TestDataFilter]
    ) -> dict[TestName, StoredTestResult]:
        """
        Get all test results for the result in the collection.

        This can be particularly expensive! Best to do it on a
        per-test basis with get_test() if possible.

        Arguments:
        ---------
        test_filter : Union[Iterable[TestDataFilter], TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        # Convert a value of list[StoredTestResult] to a single result
        return {k: v[0] for k, v in self._get_all_tests(test_filter).items()}

    def get_test(
        self,
        name: TestName,
        test_filter: Union[Iterable[TestDataFilter], TestDataFilter],
    ) -> Optional[StoredTestResult]:
        """
        Get the test result for the given test name, if any.

        Arguments:
        ---------
        name : TestName
            The name of the test.
        test_filter : Union[Iterable[TestDataFilter], TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        tests = self._get_tests(name, test_filter)
        if tests:
            assert len(tests) == 1
            return tests[0]
        return None


class ResultsCollection(ResultsCollectionBase):
    """Store multiple test results and exposes methods to loading tests."""

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
        super().__init__(results, database_getter)

    @property
    def results(self) -> list[StoredResult]:
        """Get the underlying results."""
        return self._results

    def get_all_tests(
        self, test_filter: Union[Iterable[TestDataFilter], TestDataFilter]
    ) -> dict[TestName, list[StoredTestResult]]:
        """
        Get all test results across all results in the collection.

        This can be particularly expensive! Best to do it on a
        per-test basis with get_tests() if possible.

        Arguments:
        ---------
        test_filter : Union[Iterable[TestDataFilter], TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        return self._get_all_tests(test_filter)

    def get_tests(
        self,
        name: TestName,
        test_filter: Union[Iterable[TestDataFilter], TestDataFilter],
    ) -> list[StoredTestResult]:
        """
        Get the test results for a test across all results in the collection.

        Arguments:
        ---------
        name : TestName
            The name of the test.
        test_filter : Union[Iterable[TestDataFilter], TestDataFilter]
            The TestDataFilter objects that represent which data
            to obtain for the tests.

        """
        return self._get_tests(name, test_filter)
