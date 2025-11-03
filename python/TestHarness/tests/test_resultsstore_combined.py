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

from bson.objectid import ObjectId
from test_resultsstore_civetstore import base_civet_env, random_id

from TestHarness.resultsstore.civetstore import CIVETStore
from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.storedresults import StoredResult
from TestHarness.resultsstore.utils import TestName
from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase

# Name for the database used for testing database store
TEST_DATABASE = "civet_tests_moose_resultsstore_combined"

HAS_STORE_AUTH = CIVETStore.has_authentication()
HAS_READER_AUTH = ResultsReader.hasEnvironmentAuthentication()

DEFAULT_TESTHARNESS_ARGS = ["-i", "always_ok", "--capture-perf-graph"]

TEST_NAME = TestName("tests/test_harness", "always_ok")


class TestResultsStoreCombined(TestHarnessTestCase):
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

    def deleteDocuments(self, result_id: ObjectId, test_ids: Optional[ObjectId] = None):
        """Delete documents in a database after storing them."""
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

    def compareResults(
        self, results: StoredResult, env: dict, base_sha: str, id: ObjectId
    ):
        """Compare results to the expected environment."""
        pr_num = int(env["CIVET_PR_NUM"])
        if pr_num == 0:
            pr_num = None
        event_id = int(env["CIVET_EVENT_ID"])
        head_sha = env["CIVET_HEAD_SHA"]

        self.assertEqual(results.id, id)
        self.assertEqual(results.pr_num, pr_num)
        self.assertEqual(results.event_id, event_id)
        self.assertEqual(results.base_sha, base_sha)
        self.assertEqual(results.event_sha, head_sha)
        self.assertEqual(results.num_tests, 1)
        self.assertTrue(results.has_test(TEST_NAME))
        pr_test = results.get_test(TEST_NAME)
        self.assertEqual(pr_test.results, results)
        self.assertEqual(pr_test.name, TEST_NAME)
        self.assertIsNotNone(pr_test.perf_graph)

    def runGetResultsTest(self, env: dict, base_sha: str, id: ObjectId):
        """Run run[PR,Event,Commit]Results(), comparing the result."""
        pr_num = int(env["CIVET_PR_NUM"])
        if pr_num == 0:
            pr_num = None
        event_id = int(env["CIVET_EVENT_ID"])
        head_sha = env["CIVET_HEAD_SHA"]

        for load_type in ["pr", "event", "commit"]:
            reader = ResultsReader(TEST_DATABASE)
            if load_type == "pr":
                if pr_num is None:
                    continue
                result = reader.getPRResults(pr_num)
            elif load_type == "event":
                result = reader.getEventResults(event_id)
            elif load_type == "commit":
                result = reader.getCommitResults(head_sha)
            self.assertIsNotNone(result)
            self.compareResults(result, env, base_sha, id)

    @unittest.skipUnless(HAS_STORE_AUTH, "Skipping because store auth is not available")
    @unittest.skipUnless(
        HAS_READER_AUTH, "Skipping because reader auth is not available"
    )
    def testPR(self):
        """Test storing a PR result and then loading it."""
        run_test_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS)
        result = run_test_result.results

        base_sha, base_env = base_civet_env()
        pr_num = random_id()
        env = {"CIVET_EVENT_CAUSE": "Pull request", "CIVET_PR_NUM": str(pr_num)}
        env.update(base_env)

        result_id, test_ids = CIVETStore().store(
            TEST_DATABASE, result, base_sha, env=env
        )
        self.assertIsNone(test_ids)

        try:
            # Test get[Event,PR,Commit]Results()
            self.runGetResultsTest(env, base_sha, result_id)

            # Get by test results with a PR option, from which the
            # first one should be the one from the PR
            test_reader = ResultsReader(TEST_DATABASE)
            test_results = test_reader.getTestResults(TEST_NAME, limit=1, pr_num=pr_num)
            self.assertEqual(len(test_results), 1)
            test_result = test_results[0]
            self.assertEqual(test_result.name, TEST_NAME)
            self.assertEqual(test_result.result_id, result_id)
            self.compareResults(test_result.results, env, base_sha, result_id)

        except:
            self.deleteDocuments(result_id)
            raise

        self.deleteDocuments(result_id)

    @unittest.skipUnless(HAS_STORE_AUTH, "Skipping because store auth is not available")
    @unittest.skipUnless(
        HAS_READER_AUTH, "Skipping because reader auth is not available"
    )
    def testPush(self):
        """Test storing a push event and then loading it."""
        run_test_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS)
        result = run_test_result.results

        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Push next", "CIVET_PR_NUM": "0"}
        env.update(base_env)

        result_id, test_ids = CIVETStore().store(
            TEST_DATABASE, result, base_sha, env=env
        )
        self.assertIsNone(test_ids)

        try:
            # Test get[Event,PR,Commit]Results()
            self.runGetResultsTest(env, base_sha, result_id)

            # Test getLatestPushResults(); python tests share
            # a database so we want to make sure that we have
            # at least one that matches ours
            latest_reader = ResultsReader(TEST_DATABASE)
            latest_results = latest_reader.getLatestPushResults(50)
            find_result = [r for r in latest_results if r.id == result_id]
            self.assertEqual(len(find_result), 1)
            this_result = find_result[0]
            self.compareResults(this_result, env, base_sha, result_id)

            # Test getTestResults(); same issue with python
            # tests sharing a database here too
            test_reader = ResultsReader(TEST_DATABASE)
            test_results = test_reader.getTestResults(TEST_NAME, limit=50)
            find_test_result = [r for r in test_results if r.result_id == result_id]
            self.assertEqual(len(find_test_result), 1)
            this_test_result = find_test_result[0]
            self.assertEqual(this_test_result.name, TEST_NAME)
            self.assertEqual(this_test_result.result_id, result_id)
            self.compareResults(this_test_result.results, env, base_sha, result_id)
        except:
            self.deleteDocuments(result_id)
            raise

        self.deleteDocuments(result_id)

    @unittest.skipUnless(HAS_STORE_AUTH, "Skipping because store auth is not available")
    @unittest.skipUnless(
        HAS_READER_AUTH, "Skipping because reader auth is not available"
    )
    def testSeparateTests(self):
        """Test storing an event and then loading it."""
        run_test_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS)
        result = run_test_result.results

        base_sha, base_env = base_civet_env()
        pr_num = random_id()
        env = {"CIVET_EVENT_CAUSE": "Pull request", "CIVET_PR_NUM": str(pr_num)}
        env.update(base_env)
        event_sha = env["CIVET_HEAD_SHA"]

        result_id, test_ids = CIVETStore().store(
            TEST_DATABASE, result, base_sha, env=env, max_result_size=1e-6
        )
        self.assertIsNotNone(test_ids)
        self.assertEqual(len(test_ids), 1)

        try:
            reader = ResultsReader(TEST_DATABASE)
            results = reader.getCommitResults(event_sha)
            self.assertIsNotNone(result)
            self.compareResults(results, env, base_sha, result_id)
            pr_test = results.get_test(TEST_NAME)
            self.assertEqual(pr_test.id, test_ids[0])
        except:
            self.deleteDocuments(result_id, test_ids)
            raise

        self.deleteDocuments(result_id, test_ids)


if __name__ == "__main__":
    unittest.main()
