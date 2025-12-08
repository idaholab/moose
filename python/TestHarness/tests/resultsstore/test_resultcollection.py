# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.resultcollection.ResultCollection."""

import unittest
from typing import Tuple, Union

import pytest
from bson.objectid import ObjectId
from mock import patch
from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.resultcollection import (
    ResultCollection,
    ResultsCollection,
)
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.storedtestresult import StoredTestResult
from TestHarness.resultsstore.testdatafilters import TestDataFilter
from TestHarness.resultsstore.utils import (
    TestName,
    mutable_results_test_iterator,
    results_num_tests,
    results_test_iterator,
)
from TestHarness.tests.resultsstore.common import (
    GOLD_DATABASE_NAME,
    GOLD_DATABASE_TEST_NAME,
    GOLD_RESULTS,
    TEST_DATABASE_NAME,
    TEST_DATABASE_TEST_NAME,
    FakeMongoClient,
    FakeMongoDatabase,
    FakeMongoFind,
    ResultsStoreTestCase,
)
from TestHarness.tests.resultsstore.test_storedresult import build_stored_result

# Fake database name for testing
FAKE_DATABASE_NAME = "foodb"

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
READER_AUTH = ResultsReader.load_authentication()
HAS_READER_AUTH = READER_AUTH is not None

# Fake test name for testing
TEST_NAME = TestName("foo", "bar")


def as_iterable_and_non_iterable(
    test_filter: TestDataFilter,
) -> list[Union[TestDataFilter, Tuple[TestDataFilter]]]:
    """Get an iterable and non-iterable form of the given filter."""
    return [(test_filter,), test_filter]


def all_filters() -> list[Union[TestDataFilter, Tuple[TestDataFilter]]]:
    """Get all possible filters, both in an iterable and non-iterable form."""
    values = []
    for test_filter in TestDataFilter:
        values += as_iterable_and_non_iterable(test_filter)
    return values


class TestResultCollection(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.resultcollection.ResultCollection."""

    def test_ResultsCollection_init(self):
        """Test ResultsCollection.__init__() for and basic properties/getters."""
        results = [StoredResult(self.get_result_data())]

        database = FakeMongoDatabase()

        def database_getter():
            return database

        collection = ResultsCollection(results, database_getter)

        self.assertEqual(collection._results, results)
        self.assertEqual(collection._database_getter, database_getter)

        self.assertEqual(collection.results, results)
        self.assertEqual(collection.result_ids, [v.id for v in results])
        self.assertIs(collection.get_database(), database)

    def test_ResultCollection_init(self):
        """Test ResultCollection.__init__() for and basic properties/getters."""
        result = StoredResult(self.get_result_data())

        database = FakeMongoDatabase()

        def database_getter():
            return database

        collection = ResultCollection(result, database_getter)

        self.assertEqual(len(collection._results), 1)
        self.assertEqual(collection._results[0], result)
        self.assertEqual(collection._database_getter, database_getter)

        self.assertEqual(collection.result, result)
        self.assertEqual(collection.result_ids, [result.id])
        self.assertIs(collection.get_database(), database)

    def get_gold_resultscollection(self) -> Tuple[ResultsCollection, ResultsReader]:
        """Get a ResultCollection for the gold tests."""
        reader = ResultsReader(GOLD_DATABASE_NAME, authentication=READER_AUTH)

        results = []
        for gold_result in GOLD_RESULTS:
            collection = reader.get_commit_result(gold_result.event_sha)
            assert collection is not None
            results.append(collection.result)

        return (
            ResultsCollection(
                sorted(results, key=lambda v: v.id, reverse=True), reader.get_database
            ),
            reader,
        )

    def check_filter(
        self,
        test: StoredTestResult,
        test_filter: Union[TestDataFilter, Tuple[TestDataFilter]],
    ):
        """Test if the right data is loaded in a test given a filter."""
        if isinstance(test_filter, tuple):
            test_filter = test_filter[0]
        if test_filter != TestDataFilter.ALL:
            for other_filter in TestDataFilter:
                if other_filter != test_filter:
                    self.assertNotIn(other_filter.value, test.data)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_READER_AUTH, "Reader authentication unavailable")
    def test_get_all_tests_live(self):
        """Test get_all_tests() with a live database."""
        # Test the latest two events
        with ResultsReader(TEST_DATABASE_NAME, authentication=READER_AUTH) as ctx:
            collection = ctx.reader.get_latest_push_results(2)
            assert collection is not None
            self.assertEqual(len(collection.results), 2)

            for test_filter in all_filters():
                all_tests = collection.get_all_tests(test_filter)
                self.assertIn(TEST_DATABASE_TEST_NAME, all_tests)
                for name, tests in all_tests.items():
                    if name == TEST_DATABASE_TEST_NAME:
                        self.assertEqual(len(tests), 2)
                        for test in tests:
                            self.assertEqual(test.name, TEST_DATABASE_TEST_NAME)
                            self.check_filter(test, test_filter)

        # Test the gold events
        collection, reader = self.get_gold_resultscollection()
        for test_filter in all_filters():
            all_tests = collection.get_all_tests(test_filter)
            self.assertIn(GOLD_DATABASE_TEST_NAME, all_tests)
            for name, tests in all_tests.items():
                if name == GOLD_DATABASE_TEST_NAME:
                    self.assertEqual(len(tests), len(GOLD_RESULTS))
                    for test in tests:
                        self.assertEqual(test.name, GOLD_DATABASE_TEST_NAME)
                        self.check_filter(test, test_filter)
                    # Should have at least test with separate tests
                    self.assertTrue(any(test.id is not None for test in tests))
                    # And one with combined tests
                    self.assertTrue(any(test.id is None for test in tests))
        reader.close()

    def test_get_all_tests_with_separate(self):
        """Test _get_all_tests() with separate tests."""
        # Create the first result, where the tests are stored within
        within_result_data = self.get_testharness_result()
        within_result = build_stored_result(within_result_data)
        within_data = [{"_id": within_result.id, "tests": within_result_data["tests"]}]

        # Create the second result, where the tests are stored separately
        separate_result_data = self.get_testharness_result()
        separate_result = build_stored_result(separate_result_data)
        separate_data = [
            {"_id": separate_result.id, "tests": separate_result_data["tests"]}
        ]
        separate_test_data = []
        separate_test_ids = {}
        for test in mutable_results_test_iterator(separate_data[0]):
            test_id = ObjectId()
            separate_test_ids[test.name] = test_id
            separate_test_data.append(
                {"_id": test_id, "result_id": separate_result.id, **test.value}
            )
            test.set_value(test_id)
        separate_test_data = list(reversed(separate_test_data))

        results = [within_result, separate_result]

        # Build a dummy database connection
        db = FakeMongoDatabase()
        test_filter = [TestDataFilter.ALL]
        collection = ResultsCollection(results, lambda: db)

        # Run the find and aggregation aggregation
        with (
            patch.object(
                db.results,
                "find",
                return_value=FakeMongoFind(within_data + separate_data),
            ),
            patch.object(
                db.tests, "aggregate", return_value=FakeMongoFind(separate_test_data)
            ),
        ):
            all_tests = collection.get_all_tests(test_filter)

        gold_result = self.get_testharness_result()
        num_tests = results_num_tests(gold_result)
        self.assertEqual(num_tests, len(all_tests))
        for gold_test in results_test_iterator(gold_result):
            tests = all_tests[gold_test.name]
            self.assertEqual(len(tests), 2)
            for i, test in enumerate(tests):
                self.assertEqual(test.name, gold_test.name)
                result = within_result if i == 0 else separate_result
                self.assertIs(result, test.result)
                if i == 0:  # test within
                    self.assertIsNone(test.id)
                else:  # test separate
                    self.assertEqual(test.id, separate_test_ids[test.name])

    def test_get_all_tests_no_separate(self):
        """Test _get_all_tests() without separate tests."""
        result_data = self.get_testharness_result()
        result = build_stored_result(result_data)
        find_data = [{"_id": result.id, "tests": result_data["tests"]}]

        results = [result]

        # Build a dummy database connection
        db = FakeMongoDatabase()
        test_filter = [TestDataFilter.ALL]
        collection = ResultsCollection(results, lambda: db)

        # Run the find and aggregation aggregation
        with (
            patch.object(
                db.results,
                "find",
                return_value=FakeMongoFind(find_data),
            ),
            patch.object(
                db.tests,
                "aggregate",
            ) as patch_aggregate,
        ):
            all_tests = collection.get_all_tests(test_filter)

        patch_aggregate.assert_not_called()
        num_tests = results_num_tests(result_data)
        self.assertEqual(num_tests, len(all_tests))
        for gold_test in results_test_iterator(result_data):
            tests = all_tests[gold_test.name]
            self.assertEqual(len(tests), 1)
            for test in tests:
                self.assertEqual(test.name, gold_test.name)
                self.assertIsNone(test.id)

    def test_get_tests(self):
        """Test _get_tests()."""
        # Create two results, one of which will store tests separately
        within_result = build_stored_result(self.get_testharness_result())
        separate_result = build_stored_result(self.get_testharness_result())
        results = [within_result, separate_result]

        # Build the data we'd get from mongo; the first entry is the
        # result without the separate test (no test_id) and th second
        # is the result with the sparate test
        separate_test_id = ObjectId()
        aggregate_data = [
            {"_id": within_result.id},
            {"_id": separate_result.id, "test_id": separate_test_id},
        ]

        # Build a dummy database connection
        db = FakeMongoDatabase()
        test_filter = [TestDataFilter.ALL]
        collection = ResultsCollection(results, lambda: db)
        pipeline = ResultsCollection._get_tests_pipeline(
            collection.result_ids, TEST_NAME, test_filter
        )

        # Run the aggregation
        with patch.object(
            db.results, "aggregate", return_value=FakeMongoFind(aggregate_data)
        ) as patch_aggregate:
            tests = collection.get_tests(TEST_NAME, test_filter)

        patch_aggregate.assert_called_once_with(pipeline)
        self.assertEqual(len(tests), 2)
        for i, test in enumerate(tests):
            self.assertIsInstance(test, StoredTestResult)
            self.assertEqual(test.name, TEST_NAME)
            self.assertIs(test.result, results[i])
            if i == 0:  # test within
                self.assertIsNone(test.id)
            else:  # test separate
                self.assertEqual(test.id, separate_test_id)
            # Should be removed
            self.assertNotIn("test_id", test.data)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_READER_AUTH, "Reader authentication unavailable")
    def test_get_tests_live(self):
        """Test get_tests() with a live database."""
        # Test the latest two events
        with ResultsReader(TEST_DATABASE_NAME, authentication=READER_AUTH) as ctx:
            collection = ctx.reader.get_latest_push_results(2)
            assert collection is not None
            self.assertEqual(len(collection.results), 2)

            for test_filter in all_filters():
                tests = collection.get_tests(TEST_DATABASE_TEST_NAME, test_filter)
                self.assertEqual(len(tests), 2)
                for test in tests:
                    self.assertEqual(test.name, TEST_DATABASE_TEST_NAME)
                    self.check_filter(test, test_filter)

        # Test the gold events
        collection, reader = self.get_gold_resultscollection()
        for test_filter in all_filters():
            tests = collection.get_tests(GOLD_DATABASE_TEST_NAME, test_filter)
            self.assertEqual(len(tests), len(GOLD_RESULTS))
            for test in tests:
                # Should be the test we expect
                self.assertEqual(test.name, GOLD_DATABASE_TEST_NAME)
                # Only has expected loaded data
                self.check_filter(test, test_filter)

            # Should have at least one with separate tests
            self.assertTrue(any(test.id is not None for test in tests))
            # And one with combined tests
            self.assertTrue(any(test.id is None for test in tests))
        reader.close()

    def test_ResultCollection_get_test_names(self):
        """Test ResultCollection.get_test_names()."""
        client = FakeMongoClient()
        reader = ResultsReader(FAKE_DATABASE_NAME, client)
        results = [StoredResult(self.get_result_data()) for _ in range(2)]

        database = client.get_database(FAKE_DATABASE_NAME)
        docs = [
            {"tests": [["folder1", ["test1", "test2"]]]},
        ]

        collection = ResultCollection(results[0], reader.get_database)
        with patch.object(
            database.results, "aggregate", return_value=FakeMongoFind(docs)
        ):
            collection_names = collection.get_test_names()
        self.assertEqual(
            collection_names,
            set(
                [
                    TestName("folder1", "test1"),
                    TestName("folder1", "test2"),
                ]
            ),
        )

    def test_ResultsCollection_get_test_names(self):
        """Test ResultsCollection.get_test_names()."""
        client = FakeMongoClient()
        reader = ResultsReader(FAKE_DATABASE_NAME, client)
        results = [StoredResult(self.get_result_data()) for _ in range(2)]

        database = client.get_database(FAKE_DATABASE_NAME)
        docs = [
            {"tests": [["folder1", ["test1", "test2"]]]},
            {"tests": [["folder1", ["test1"]], ["folder2", ["test1"]]]},
        ]

        collection = ResultsCollection(results, reader.get_database)
        with patch.object(
            database.results, "aggregate", return_value=FakeMongoFind(docs)
        ):
            collection_names = collection.get_test_names()
        self.assertEqual(
            collection_names,
            set(
                [
                    TestName("folder1", "test1"),
                    TestName("folder1", "test2"),
                    TestName("folder2", "test1"),
                ]
            ),
        )

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_READER_AUTH, "Reader authentication unavailable")
    def test_get_test_names_live(self):
        """Test get_test_names() with a live database."""
        # Test the latest events
        with ResultsReader(TEST_DATABASE_NAME, authentication=READER_AUTH) as ctx:
            collection = ctx.reader.get_latest_push_results(2)
            assert collection is not None
            self.assertEqual(len(collection.results), 2)
            names = collection.get_test_names()
            self.assertEqual(names, set([TEST_DATABASE_TEST_NAME]))

        # Test the gold events
        collection, reader = self.get_gold_resultscollection()
        names = collection.get_test_names()
        reader.close()
        self.assertIn(GOLD_DATABASE_TEST_NAME, names)

    def test_ResultCollection_get_all_tests(self):
        """Test ResultCollection.get_all_tests() calling the parent method."""
        result = StoredResult(self.get_result_data())
        collection = ResultCollection(result, lambda: None)

        # Filter as non-iterable and iterable
        for test_filter in as_iterable_and_non_iterable(TestDataFilter.ALL):
            with patch.object(
                collection,
                "_get_all_tests",
                return_value={"foo": ["bar"], "baz": ["bang"]},
            ) as patch_get_all_tests:
                tests = collection.get_all_tests(test_filter)
            patch_get_all_tests.assert_called_once_with(test_filter)
            self.assertEqual(tests, {"foo": "bar", "baz": "bang"})

    def test_ResultCollection_get_test(self):
        """Test ResultCollection.get_test() calling the parent method."""
        result = StoredResult(self.get_result_data())
        collection = ResultCollection(result, lambda: None)

        # Filter as non-iterable and iterable
        for test_filter in as_iterable_and_non_iterable(TestDataFilter.ALL):
            # No test
            with patch.object(
                collection,
                "_get_tests",
                return_value=None,
            ) as patch_get_tests:
                test = collection.get_test(TEST_NAME, test_filter)
            patch_get_tests.assert_called_once_with(TEST_NAME, test_filter)
            self.assertIsNone(test)

            # One test
            with patch.object(
                collection,
                "_get_tests",
                return_value=["foo"],
            ) as patch_get_tests:
                test = collection.get_test(TEST_NAME, test_filter)
            patch_get_tests.assert_called_once_with(TEST_NAME, test_filter)
            self.assertEqual(test, "foo")

    def test_ResultsCollection_get_all_tests(self):
        """Test ResultsCollection.get_all_tests() calling the parent method."""
        result = StoredResult(self.get_result_data())
        collection = ResultsCollection([result], lambda: None)

        # Filter as non-iterable and iterable
        for test_filter in as_iterable_and_non_iterable(TestDataFilter.ALL):
            with patch.object(
                collection, "_get_all_tests", return_value="foo"
            ) as patch_get_all_tests:
                value = collection.get_all_tests(test_filter)
            patch_get_all_tests.assert_called_once_with(test_filter)
            self.assertEqual(value, "foo")

    def test_ResultsCollection_get_tests(self):
        """Test ResultsCollection.get_all_tests() calling the parent method."""
        result = StoredResult(self.get_result_data())
        collection = ResultsCollection([result], lambda: None)

        # Filter as non-iterable and iterable
        for test_filter in as_iterable_and_non_iterable(TestDataFilter.ALL):
            with patch.object(
                collection, "_get_tests", return_value="foo"
            ) as patch_get_all_tests:
                value = collection.get_tests(TEST_NAME, test_filter)

            patch_get_all_tests.assert_called_once_with(TEST_NAME, test_filter)
            self.assertEqual(value, "foo")
