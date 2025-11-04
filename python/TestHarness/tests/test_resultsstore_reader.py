# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.resultsreader.ResultsReader."""

import json
import os
import unittest
from collections import OrderedDict
from copy import deepcopy
from dataclasses import dataclass
from typing import Optional

from bson.objectid import ObjectId
from mock import patch
from pymongo import MongoClient

from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.storedresults import StoredResult, StoredTestResult
from TestHarness.resultsstore.utils import (
    TestName,
    compress_dict,
    decompress,
    results_test_iterator,
)

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = ResultsReader.hasEnvironmentAuthentication()

# Production database file for testing get_test_results
PROD_GET_TEST_RESULTS_GOLD_PATH = os.path.join(
    os.path.dirname(__file__), "gold", "resultsstore", "prod_get_test_results.json"
)
# Production database name for testing real results
PROD_DATABASE_NAME = "civet_tests_moose_performance"
# The name of the test to load from the production database
PROD_TEST_NAME = TestName("simple_transient_diffusion", "test")

# Test database name for testing pull request results
TEST_DATABASE_NAME = "civet_tests_moose_store_results_live"
# The name of the test to load from the test database
TEST_TEST_NAME = TestName("tests/test_harness", "ok")


class FakeMongoClient(MongoClient):
    """Fake MongoClient for unit testing."""

    def __init__(self, *args, **kwargs):
        """Empty initialize."""
        pass

    def list_database_names(self):
        """Get mocked database names."""
        return [PROD_DATABASE_NAME]

    def get_database(self, name):
        """Get a mocked database."""
        assert name == PROD_DATABASE_NAME

    def close(self):
        """Close; don't do anything."""
        pass


class TestResultsReader(unittest.TestCase):
    """Test TestHarness.resultsstore.resultsreader.ResultsReader."""

    @staticmethod
    def replaceJSONMetadata(results: dict):
        """Simplify tester json_metadata to avoid storing excessive content."""
        for test in results_test_iterator(results):
            if (tester := test.value.get("tester")) is not None and (
                json_metadata := tester.get("json_metadata")
            ) is not None:
                tester["json_metadata"] = {
                    key: decompress(compress_dict({"fake_metadata_for": key}))
                    for key in json_metadata
                }

    def buildProdGetTestResultsGold(self):
        """Build a gold file for for testing, using real data."""

        @dataclass
        class GoldTest:
            civet_version: int
            event_sha: Optional[str] = None
            event_id: Optional[int] = None

        # Static set of results from which to build the gold file
        gold_tests = [
            GoldTest(
                civet_version=0, event_sha="968f537a3c89ffb556fb7da18da28da52b592ee0"
            ),
            GoldTest(
                civet_version=0, event_sha="3d48fa82c081e141fd86390dfb9edd1f11c984ca"
            ),
            GoldTest(
                civet_version=1, event_sha="45b8a536530388e7bb1ff563398b1e94f2d691fc"
            ),
            # bump to civet_version=2
            GoldTest(
                civet_version=2, event_sha="1134c7e383972783be2ea702f2738ece79fe6a59"
            ),
            # bump to civet_version=3
            # - added event_id
            GoldTest(civet_version=3, event_id=258481),
            # bump to civet_version=4
            # - remove indices from tests
            GoldTest(civet_version=4, event_id=259309),
            # bump to civet_version=5
            GoldTest(civet_version=5, event_id=260038),
            # bump to civet_version=6
            # - tests can be stored with results
            # - store moved to CIVETStore object
            GoldTest(civet_version=6, event_id=260206),
        ]

        # This can be set to true once to overwrite the gold file
        rewrite_gold = True

        with ResultsReader(PROD_DATABASE_NAME) as ctx:
            reader = ctx.reader
            # Load each result, where gold is indexed from
            # result id -> result data
            gold = {}
            for gold_test in gold_tests:
                results = None
                if gold_test.event_sha:
                    results = reader.getCommitResults(gold_test.event_sha)
                else:
                    self.assertIsNotNone(gold_test.event_id)
                    results = reader.getEventResults(gold_test.event_id)
                self.assertIsNotNone(results)
                self.assertTrue(results.has_test(PROD_TEST_NAME))

                # Serialize result so tha we can store it in a file
                serialized = results.serialize(test_filter=[PROD_TEST_NAME])

                # Replace JSON metadata with something smaller so that
                # we don't need to store a large amount of data
                self.replaceJSONMetadata(serialized)

                # Store for output
                gold[str(results.id)] = serialized

                # Checks
                self.assertEqual(results.civet_version, gold_test.civet_version)
                if gold_test.event_sha:
                    self.assertEqual(results.event_sha, gold_test.event_sha)
                elif gold_test.event_id:
                    self.assertEqual(results.event_id, gold_test.event_id)

        # Dump the values so that we can load them
        gold_dumped = json.dumps(gold, indent=2, sort_keys=True)

        # Rewrite the gold file if we set to do so
        if rewrite_gold:
            with open(PROD_GET_TEST_RESULTS_GOLD_PATH, "w") as f:
                f.write(gold_dumped)

        return gold_dumped

    def testHasAuth(self):
        """Check if auth is available or not given environment variables."""
        if os.environ.get("TEST_RESULTSREADER_HAS_AUTH") is not None:
            self.assertTrue(HAS_AUTH)
        if os.environ.get("TEST_RESULTSREADER_MISSING_AUTH") is not None:
            self.assertFalse(HAS_AUTH)

    def testInitNoCheck(self):
        """Test setting check=False in __init__()."""
        reader = ResultsReader("unused", client=FakeMongoClient(), check=False)
        self.assertFalse(reader.check)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testGetTestResultsGold(self):
        """Generate the gold file using the live server and compare the result."""
        new_gold = self.buildProdGetTestResultsGold()

        with open(PROD_GET_TEST_RESULTS_GOLD_PATH, "r") as f:
            gold = json.load(f)

        # Augment JSON metadata that was changed in the gold store
        for result in gold.values():
            self.replaceJSONMetadata(result)

        self.assertEqual(gold, json.loads(new_gold))

    def _testGetTestResults(
        self, reader: ResultsReader, **kwargs
    ) -> list[StoredTestResult]:
        """Runner for testing getTestResults()."""
        results = reader.getTestResults(PROD_TEST_NAME, **kwargs)

        last_id = None
        last_time = None

        for result in results:
            self.assertIsInstance(result, StoredTestResult)

            # Test that result is the correct one
            test_harness_results = result.results
            self.assertEqual(test_harness_results, result.results)
            self.assertEqual(PROD_TEST_NAME, result.name)
            self.assertIsInstance(test_harness_results, StoredResult)

            # Test basic state for results header
            self.assertRegex(
                test_harness_results.civet_job_url, r"civet.inl.gov/job/\d+"
            )
            self.assertEqual(test_harness_results.event_cause, "push")

            # Event ID as of civet version 3
            if result.results.civet_version > 2:
                self.assertTrue(isinstance(test_harness_results.event_id, int))
            else:
                self.assertIsNone(test_harness_results.event_id)

            # Should have CIVET state
            self.assertEqual(result.event_cause, "push")
            self.assertIsNone(result.pr_num)

            # Test that basic entries have values
            self.assertGreater(result.run_time, 0)
            self.assertIsNotNone(result.hpc_queued_time)

            # Make sure we have the latest ordering
            if last_id is not None:
                self.assertGreater(last_id, result.id)
            if last_time is not None:
                self.assertGreater(last_time, result.time)

            last_id = result.id
            last_time = result.time

        return results

    @staticmethod
    def deserializeGold() -> dict[ObjectId, dict]:
        """
        Deserialize the gold data into a dict of result ID -> data.

        Sorted by ID, thus latest event first.
        """
        with open(PROD_GET_TEST_RESULTS_GOLD_PATH, "r") as f:
            gold = json.load(f)
        gold = {
            ObjectId(id): StoredResult.deserialize(entry) for id, entry in gold.items()
        }
        return dict(OrderedDict(sorted(gold.items(), key=lambda x: x[0], reverse=True)))

    @patch.object(ResultsReader, "_findResults")
    @patch.object(StoredResult, "_find_test_data")
    def testGetTestResultsTestsSeparate(self, patch_find_test_data, patch_find_results):
        """Test getRestResults() with mocked database calls and separate tests."""
        gold = self.deserializeGold()

        tests: dict[ObjectId, dict] = {}
        # Remove the tests entries and store them as IDs instead
        for result_entry in gold.values():
            for test in results_test_iterator(result_entry):
                test_entry = test.value
                if "_id" in test_entry:
                    tests[test_entry["_id"]] = deepcopy(test_entry)
                    test.set_value(test_entry["_id"])

        # Mock getting a test from mongodb
        def find_test_data(id):
            return tests[id]

        patch_find_test_data.side_effect = find_test_data

        # Mock getting the results from mongodb
        def get_results_data(*args, **kwargs):
            filter = args[0]
            if "_id" not in filter:
                return list(gold.values())
            return []

        patch_find_results.side_effect = get_results_data

        with ResultsReader(PROD_DATABASE_NAME, FakeMongoClient()) as ctx:
            reader = ctx.reader
            test_results = self._testGetTestResults(reader)

        # Make sure we have a test entry for each gold test, in the right order
        self.assertEqual(len(test_results), len(gold))
        for i, entry in enumerate(gold):
            self.assertEqual(entry, test_results[i].result_id)

    @patch.object(ResultsReader, "_findResults")
    def testGetTestResultsTestsIncluded(self, patch_find_results):
        """Test getRestResults() with mocked database calls and combined tests."""
        gold = self.deserializeGold()

        # Delete the ID and result_id in the tests if they exist
        for result_entry in gold.values():
            for test in results_test_iterator(result_entry):
                test_entry = test.value
                for key in ["_id", "result_id"]:
                    if key in test_entry:
                        del test_entry[key]

        # Mock getting the results from mongodb
        def get_results_data(*args, **kwargs):
            filter = args[0]
            if "_id" not in filter:
                return list(gold.values())
            return []

        patch_find_results.side_effect = get_results_data

        with ResultsReader(PROD_DATABASE_NAME, FakeMongoClient()) as ctx:
            reader = ctx.reader
            test_results = self._testGetTestResults(reader)

        # Make sure we have a test entry for each gold test, in the right order
        self.assertEqual(len(test_results), len(gold))
        for i, entry in enumerate(gold):
            self.assertEqual(entry, test_results[i].result_id)

    @patch.object(ResultsReader, "_findResults")
    def testGetTestResultsWithLimit(self, patch_find_results):
        """Test getTestResults() with a limit."""
        gold = self.deserializeGold()

        # Mock getting the results from mongodb
        def get_results_data(*args, **kwargs):
            filter = args[0]
            if "_id" not in filter:
                return list(gold.values())
            return []

        patch_find_results.side_effect = get_results_data

        with ResultsReader(PROD_DATABASE_NAME, FakeMongoClient()) as ctx:
            reader = ctx.reader
            test_results = self._testGetTestResults(reader, limit=2)

        # Make sure we have only the last two entries (the latest)
        self.assertEqual(len(test_results), 2)
        for i, entry in enumerate(list(gold.keys())[:2]):
            self.assertEqual(entry, test_results[i].result_id)

        # We should have loaded at most 10 results, even though we
        # only wanted two
        last_i = min(10, len(gold)) - 1
        last_result_id = list(gold.keys())[last_i]
        self.assertEqual(reader._last_latest_push_event_id, last_result_id)
        self.assertEqual(len(reader._latest_push_results), last_i + 1)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testGetTestResultsLive(self):
        """Test getTestResults() using the real server, if available."""
        with ResultsReader(PROD_DATABASE_NAME) as ctx:
            reader = ctx.reader

            for limit in [1, 2, 3]:
                results = self._testGetTestResults(reader, limit=limit)
                self.assertEqual(len(results), limit)
                self.assertEqual(len(reader._latest_push_results), limit)

    @unittest.skipIf(HAS_AUTH, "Skipping because authentication is available")
    def testMissingClient(self):
        """Tests creating the ResultsReader without a client/auth."""
        with self.assertRaisesRegex(ValueError, "Must specify"):
            ResultsReader("unused")

    def testInvalidClient(self):
        """Test creating the ResultsReader without a valid client/auth."""
        with self.assertRaisesRegex(TypeError, "Invalid type for 'client'"):
            ResultsReader("unused", "bad_client")

    def testMissingDatabase(self):
        """Tests creating the ResultsReader with an invalid database."""

        class BadDatabaseClient(FakeMongoClient):
            def list_database_names(self):
                return ["foo"]

        reader = ResultsReader(PROD_DATABASE_NAME, BadDatabaseClient())
        with self.assertRaisesRegex(
            ValueError, f"Database {PROD_DATABASE_NAME} not found"
        ):
            reader.database

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testMissingDatabaseLive(self):
        """Test creating the ResultsReader with a database that isn't found."""
        name = "foo1234"
        reader = ResultsReader(name)
        with self.assertRaisesRegex(ValueError, f"Database {name} not found"):
            reader.database

    @unittest.skipUnless(
        os.environ.get("TEST_RESULTSREADER_READER"),
        "Skipping because TEST_RESULTSREADER_READER not set",
    )
    def testGetTestsWithPRLive(self):
        """
        Tests getTestsResults when we have PR/event data in the test database.

        This is explicitly tested with CIVET, where a step before this
        or a dependency is able to contribute to TEST_DATABASE_NAME
        """
        self.assertTrue(HAS_AUTH)

        head_sha = os.environ.get("CIVET_HEAD_SHA")
        self.assertIsNotNone(head_sha)
        self.assertEqual(len(head_sha), 40)

        event_cause = os.environ.get("CIVET_EVENT_CAUSE")
        self.assertIsNotNone(event_cause)
        is_pr = event_cause.startswith("Pull")
        pr_num = None

        event_id = os.environ.get("CIVET_EVENT_ID")
        self.assertIsNotNone(event_id)
        event_id = int(event_id)

        job_id = os.environ.get("CIVET_JOB_ID")
        self.assertIsNotNone(job_id)
        job_id = int(job_id)

        if is_pr:
            pr_num = os.environ.get("CIVET_PR_NUM")
            self.assertIsNotNone("CIVET_PR_NUM")
            pr_num = int(pr_num)

        with ResultsReader(TEST_DATABASE_NAME) as ctx:
            reader = ctx.reader
            results = reader.getTestResults(TEST_TEST_NAME, pr_num=pr_num)
        self.assertGreater(len(results), 0)

        # Make sure we have the right test
        for result in results:
            self.assertEqual(result.name, TEST_TEST_NAME)

        # Check the latest result
        result = results[0]
        self.assertEqual(result.event_id, event_id)
        # If we're on a PR, the latest result should
        # be the data from the PR
        if is_pr:
            self.assertEqual(result.pr_num, pr_num)
            self.assertEqual(result.event_sha, head_sha)
            self.assertEqual(result.event_cause, "pr")
        # If we're not on a PR, the latest data
        # should be a push event
        else:
            self.assertIsNone(result.pr_num)
            self.assertEqual(result.event_cause, "push")

            event_results = [r for r in results if r.event_sha == head_sha]
            self.assertEqual(len(event_results), 1)

        # Make sure that every event is a push event, except
        # if we're on a PR in which all but the first should
        # be a push event and that all of the events should
        # be in decreasing id (newer to older)
        start_index = 1 if is_pr else 0
        last_id = None
        for result in results[start_index:]:
            self.assertNotEqual(result.event_cause, "pr")
            if last_id is not None:
                self.assertGreater(last_id, result.id)
            last_id = result.id

    @unittest.skipUnless(
        os.environ.get("TEST_RESULTSREADER_READER"),
        "Skipping because TEST_RESULTSREADER_READER not set",
    )
    def testGetEventResultsLive(self):
        """
        Test getEventResults() when we have PR/event data in the test database.

        This is explicitly tested with CIVET, where a step before this
        or a dependency is able to contribute to TEST_DATABASE_NAME
        """
        self.assertTrue(HAS_AUTH)

        event_id = os.environ.get("CIVET_EVENT_ID")
        self.assertIsNotNone(event_id)
        event_id = int(event_id)

        with ResultsReader(TEST_DATABASE_NAME) as ctx:
            reader = ctx.reader
            results = reader.getEventResults(event_id)
            self.assertIsNotNone(results)
            self.assertEqual(results.event_id, event_id)

            self.assertEqual(len(results.test_names), 1)
            test_results = results.get_test(TEST_TEST_NAME)
            self.assertIsNotNone(test_results)
            self.assertEqual(test_results.name, TEST_TEST_NAME)

    @unittest.skipUnless(
        os.environ.get("TEST_RESULTSREADER_READER"),
        "Skipping because TEST_RESULTSREADER_READER not set",
    )
    def testGetCommitResultsLive(self):
        """
        Test getCommitResults() when we have PR/event data in the test database.

        This is explicitly tested with CIVET, where a step before this
        or a dependency is able to contribute to TEST_DATABASE_NAME
        """
        self.assertTrue(HAS_AUTH)

        head_sha = os.environ.get("CIVET_HEAD_SHA")
        self.assertIsNotNone(head_sha)

        with ResultsReader(TEST_DATABASE_NAME) as ctx:
            reader = ctx.reader
            results = reader.getCommitResults(head_sha)
            self.assertIsNotNone(results)
            self.assertEqual(results.event_sha, head_sha)

            self.assertEqual(len(results.test_names), 1)
            test_results = results.get_test(TEST_TEST_NAME)
            self.assertIsNotNone(test_results)
            self.assertEqual(test_results.name, TEST_TEST_NAME)


if __name__ == "__main__":
    unittest.main()
