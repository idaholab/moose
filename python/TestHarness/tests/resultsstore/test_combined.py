# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the full workflow storing and reading result data."""

import unittest
from typing import Optional

import pytest
from bson.objectid import ObjectId
from TestHarness.resultsstore.civetstore import CIVETStore
from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.resultcollection import ResultCollection
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.testdatafilters import TestDataFilter
from TestHarness.tests.resultsstore.common import (
    TEST_NAMES,
    ResultsStoreTestCase,
    base_civet_env,
    random_id,
)

# Name for the database used for testing database store
TEST_DATABASE = "civet_tests_moose_resultsstore_combined"

# Authentication for storer
STORE_AUTH = CIVETStore.load_authentication()
HAS_STORE_AUTH = STORE_AUTH is not None
# Authentication for the reader
READER_AUTH = ResultsReader.load_authentication()
HAS_READER_AUTH = READER_AUTH is not None

# Default arguments for getting test results
DEFAULT_TESTHARNESS_ARGS = ["--capture-perf-graph"]


class TestResultsStoreCombined(ResultsStoreTestCase):
    """
    Tests the full workflow for storing and reading result data.

    The CIVETStore object is used to store result data (produced
    by here by a TestHarness execution) and then the ResultsReaer
    object is used to load the data that was just stored.

    Sad caveat: tests that run here share a database, which means
    that if this is running in multiple jobs at once, we could
    have some contention. Thus, we rely on randomly generated
    data (event id, PR number, SHAs, etc) to let these tests
    run (kind of) independently. The only time this causes a real
    problem is when getting "latest event data", because there could
    be multiple events in the database at the same time. Thus,
    we just make sure that we can find our event somewhere in the
    list (not necessarily the absolute latest). Lastly, we do
    clean up immediately after testing every entry that is added
    to the database to try to minimize contention.
    """

    def delete_documents(
        self, result_id: Optional[ObjectId], test_ids: Optional[list[ObjectId]] = None
    ):
        """Delete documents in a database after storing them."""
        if result_id is None:
            return

        with CIVETStore.setup_client() as client:
            db = client[TEST_DATABASE]

            result_filter = {"_id": {"$eq": result_id}}
            result_deleted = db.results.delete_one(result_filter)
            self.assertTrue(result_deleted.acknowledged)
            self.assertEqual(result_deleted.deleted_count, 1)

            if test_ids:
                tests_filter = {"_id": {"$in": test_ids}}
                tests_deleted = db.tests.delete_many(tests_filter)
                self.assertTrue(tests_deleted.acknowledged)
                self.assertEqual(tests_deleted.deleted_count, len(test_ids))

    def compare_result(self, result: StoredResult, env: dict, base_sha: str):
        """Compare a StoredResult with the civet environment."""
        pr_num = int(env["CIVET_PR_NUM"])
        if pr_num == 0:
            pr_num = None
        event_id = int(env["CIVET_EVENT_ID"])
        head_sha = env["CIVET_HEAD_SHA"]

        self.assertEqual(result.pr_num, pr_num)
        self.assertEqual(result.event_id, event_id)
        self.assertEqual(result.base_sha, base_sha)
        self.assertEqual(result.event_sha, head_sha)

    def compare_collection(
        self,
        collection: Optional[ResultCollection],
        env: dict,
        base_sha: str,
    ):
        """Compare results to the expected environment."""
        # Compare results
        assert collection is not None
        self.compare_result(collection.result, env, base_sha)

        # Get each test with all filter types
        for test_name in TEST_NAMES:
            for filter in TestDataFilter:
                test = collection.get_test(test_name, (filter,))
                self.assertIsNotNone(test)

        # Get all of the tests
        for filter in TestDataFilter:
            all_tests = collection.get_all_tests((filter,))
            self.assertEqual(set(all_tests.keys()), set(TEST_NAMES))

    def run_get_cached_result_test(self, env: dict, base_sha: str, id: ObjectId):
        """Run run_[pr,event,commit]_result(), comparing the result."""
        pr_num = int(env["CIVET_PR_NUM"])
        if pr_num == 0:
            pr_num = None
        event_id = int(env["CIVET_EVENT_ID"])
        head_sha = env["CIVET_HEAD_SHA"]

        for load_type in ["pr", "event", "commit"]:
            with ResultsReader(TEST_DATABASE, authentication=READER_AUTH) as ctx:
                reader = ctx.reader
                collection = None
                if load_type == "pr":
                    if pr_num is None:
                        continue
                    collection = reader.get_pr_result(pr_num)
                elif load_type == "event":
                    collection = reader.get_event_result(event_id)
                elif load_type == "commit":
                    collection = reader.get_commit_result(head_sha)
                self.assertIsNotNone(collection)
                self.compare_collection(collection, env, base_sha)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_STORE_AUTH, "Store auth unavailable")
    @unittest.skipUnless(HAS_READER_AUTH, "Reader auth unavailable")
    def test_pr(self):
        """Test storing a PR result and then loading it."""
        data = self.get_testharness_result(*DEFAULT_TESTHARNESS_ARGS)

        base_sha, base_env = base_civet_env()
        pr_num = random_id()
        env = {"CIVET_EVENT_CAUSE": "Pull request", "CIVET_PR_NUM": str(pr_num)}
        env.update(base_env)

        result_id = None
        try:
            result_id, test_ids = CIVETStore().store(
                TEST_DATABASE, data, base_sha, env=env
            )
            self.assertIsInstance(result_id, ObjectId)
            self.assertIsNone(test_ids)

            # Test get_[event,pr,commit]_result()
            self.run_get_cached_result_test(env, base_sha, result_id)
        finally:
            self.delete_documents(result_id)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_STORE_AUTH, "Store auth unavailable")
    @unittest.skipUnless(HAS_READER_AUTH, "Reader auth unavailable")
    def test_push(self):
        """Test storing a push event and then loading it."""
        data = self.get_testharness_result(*DEFAULT_TESTHARNESS_ARGS)

        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Push next", "CIVET_PR_NUM": "0"}
        env.update(base_env)

        result_id = None
        test_ids = None
        try:
            result_id, test_ids = CIVETStore().store(
                TEST_DATABASE, data, base_sha, env=env
            )
            self.assertIsInstance(result_id, ObjectId)
            self.assertIsNone(test_ids)

            # Test get_[event,pr,commit]_result()
            self.run_get_cached_result_test(env, base_sha, result_id)

            # Test get_latest_push_results(); python tests share
            # a database so we want to make sure that we have
            # at least one that matches ours
            with ResultsReader(TEST_DATABASE, authentication=READER_AUTH) as ctx:
                reader = ctx.reader
                latest_collection = reader.get_latest_push_results(50)
                assert latest_collection is not None
                find_result = [
                    r for r in latest_collection.results if r.id == result_id
                ]
                self.assertEqual(len(find_result), 1)
                this_result = find_result[0]
                self.compare_result(this_result, env, base_sha)
        finally:
            self.delete_documents(result_id)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_STORE_AUTH, "Store auth unavailable")
    @unittest.skipUnless(HAS_READER_AUTH, "Reader auth unavailable")
    def test_separate_tests(self):
        """Test storing an event with separate tests and then loading it."""
        data = self.get_testharness_result(*DEFAULT_TESTHARNESS_ARGS)

        base_sha, base_env = base_civet_env()
        pr_num = random_id()
        env = {"CIVET_EVENT_CAUSE": "Pull request", "CIVET_PR_NUM": str(pr_num)}
        env.update(base_env)
        event_sha = env["CIVET_HEAD_SHA"]

        result_id = None
        test_ids = None
        try:
            result_id, test_ids = CIVETStore().store(
                TEST_DATABASE, data, base_sha, env=env, max_result_size=1e-6
            )
            assert test_ids is not None
            self.assertEqual(len(test_ids), len(TEST_NAMES))

            with ResultsReader(TEST_DATABASE, authentication=READER_AUTH) as ctx:
                reader = ctx.reader
                collection = reader.get_commit_result(event_sha)
                assert collection is not None
                self.compare_collection(collection, env, base_sha)
        finally:
            self.delete_documents(result_id, test_ids)
