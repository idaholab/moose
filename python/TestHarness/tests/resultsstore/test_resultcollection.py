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
from typing import Tuple

import pytest
from mock import patch
from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.resultcollection import ResultCollection
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.storedtestresult import StoredTestResult
from TestHarness.resultsstore.testdatafilters import TestDataFilter
from TestHarness.resultsstore.utils import TestName
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

# Fake database name for testing
FAKE_DATABASE_NAME = "foodb"

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
READER_AUTH = ResultsReader.load_authentication()
HAS_READER_AUTH = READER_AUTH is not None


class TestResultCollection(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.resultcollection.ResultCollection."""

    def test_init(self):
        """Test __init__() and basic properties/getters."""
        results = [StoredResult(self.get_result_data())]

        database = FakeMongoDatabase()

        def database_getter():
            return database

        collection = ResultCollection(results, database_getter)

        self.assertEqual(collection._results, results)
        self.assertEqual(collection._database_getter, database_getter)

        self.assertEqual(collection.results, results)
        self.assertEqual(collection.result_ids, [v.id for v in results])
        self.assertEqual(id(collection.get_database()), id(database))

    def get_gold_collection(self) -> Tuple[ResultCollection, ResultsReader]:
        """Get a ResultCollection for the gold tests."""
        reader = ResultsReader(GOLD_DATABASE_NAME, authentication=READER_AUTH)

        results = []
        for gold_result in GOLD_RESULTS:
            collection = reader.get_commit_result(gold_result.event_sha)
            assert collection is not None
            self.assertEqual(len(collection.results), 1)
            results.append(collection.results[0])

        return (
            ResultCollection(
                sorted(results, key=lambda v: v.id, reverse=True), reader.get_database
            ),
            reader,
        )

    def check_filter(self, test: StoredTestResult, test_filter: TestDataFilter):
        """Test if the right data is loaded in a test given a filter."""
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

            for test_filter in TestDataFilter:
                all_tests = collection.get_all_tests((test_filter,))
                self.assertIn(TEST_DATABASE_TEST_NAME, all_tests)
                for name, tests in all_tests.items():
                    if name == TEST_DATABASE_TEST_NAME:
                        self.assertEqual(len(tests), 2)
                        for test in tests:
                            self.assertEqual(test.name, TEST_DATABASE_TEST_NAME)
                            self.check_filter(test, test_filter)

        # Test the gold events
        collection, reader = self.get_gold_collection()
        for test_filter in TestDataFilter:
            all_tests = collection.get_all_tests((test_filter,))
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

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_READER_AUTH, "Reader authentication unavailable")
    def test_get_tests_live(self):
        """Test get_tests() with a live database."""
        # Test the latest two events
        with ResultsReader(TEST_DATABASE_NAME, authentication=READER_AUTH) as ctx:
            collection = ctx.reader.get_latest_push_results(2)
            assert collection is not None
            self.assertEqual(len(collection.results), 2)

            for test_filter in TestDataFilter:
                tests = collection.get_tests(TEST_DATABASE_TEST_NAME, (test_filter,))
                self.assertEqual(len(tests), 2)
                for test in tests:
                    self.assertEqual(test.name, TEST_DATABASE_TEST_NAME)

                    self.check_filter(test, test_filter)

        # Test the gold events
        collection, reader = self.get_gold_collection()
        for test_filter in TestDataFilter:
            tests = collection.get_tests(GOLD_DATABASE_TEST_NAME, (test_filter,))
            self.assertEqual(len(tests), len(GOLD_RESULTS))
            for test in tests:
                # Should be the test we expect
                self.assertEqual(test.name, GOLD_DATABASE_TEST_NAME)
                # Only has expected loaded data
                self.check_filter(test, test_filter)

            # Should have at least test with separate tests
            self.assertTrue(any(test.id is not None for test in tests))
            # And one with combined tests
            self.assertTrue(any(test.id is None for test in tests))
        reader.close()

    def test_get_test_names(self):
        """Test get_test_names()."""
        client = FakeMongoClient()
        reader = ResultsReader(FAKE_DATABASE_NAME, client)
        results = [StoredResult(self.get_result_data()) for _ in range(2)]

        database = client.get_database(FAKE_DATABASE_NAME)
        docs = [
            {"tests": [["folder1", ["test1", "test2"]]]},
            {"tests": [["folder1", ["test1"]], ["folder2", ["test1"]]]},
        ]

        collection = ResultCollection(results, reader.get_database)
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
        collection, reader = self.get_gold_collection()
        names = collection.get_test_names()
        reader.close()
        self.assertIn(GOLD_DATABASE_TEST_NAME, names)
