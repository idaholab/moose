#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest

from bson.objectid import ObjectId
from typing import Optional

from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase
from TestHarness.resultsstore.civetstore import CIVETStore
from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.storedresults import StoredResult, TestName

from test_resultsstore_civetstore import TEST_DATABASE, base_civet_env, random_id

HAS_STORE_AUTH = CIVETStore.has_authentication()
HAS_READER_AUTH = ResultsReader.hasEnvironmentAuthentication()

DEFAULT_TESTHARNESS_ARGS = ['-i', 'always_ok', '--capture-perf-graph']

TEST_NAME = TestName('tests/test_harness', 'always_ok')

class TestResultsStoreCombined(TestHarnessTestCase):
    def deleteDocuments(self, result_id: ObjectId, test_ids: Optional[ObjectId] = None):
        """
        Helper for deleting documents in a database
        after storing into it
        """
        with CIVETStore.setup_client() as client:
            db = client[TEST_DATABASE]

            result_filter = {'_id': {'$eq': result_id}}
            result_deleted = db.results.delete_one(result_filter)
            self.assertTrue(result_deleted.acknowledged)
            self.assertEqual(result_deleted.deleted_count, 1)

            if test_ids:
                tests_filter = {'_id': {'$in': test_ids}}
                tests_deleted = db.tests.delete_many(tests_filter)
                self.assertTrue(tests_deleted.acknowledged)
                self.assertEqual(tests_deleted.deleted_count, len(test_ids))

    def compareResults(self, results: StoredResult, env: dict, base_sha: str, id: ObjectId):
        """
        Helper for comparing results to the expected environment
        """
        pr_num = int(env['CIVET_PR_NUM'])
        if pr_num == 0:
            pr_num = None
        event_id = int(env['CIVET_EVENT_ID'])
        head_sha = env['CIVET_HEAD_SHA']

        self.assertEqual(results.id, id)
        self.assertEqual(results.pr_num, pr_num)
        self.assertEqual(results.event_id, event_id)
        self.assertEqual(results.base_sha, base_sha)
        self.assertEqual(results.event_sha, head_sha)
        self.assertEqual(results.num_tests, 1)
        self.assertTrue(results.has_test(TEST_NAME.folder, TEST_NAME.name))
        pr_test = results.get_test(TEST_NAME.folder, TEST_NAME.name)
        self.assertEqual(pr_test.results, results)
        self.assertEqual(pr_test.name, TEST_NAME)
        self.assertIsNotNone(pr_test.perf_graph)

    def runGetResultsTest(self, env: dict, base_sha: str, id: ObjectId):
        """
        Helper for running run[PR,Event,Commit]Results() for the given
        environment, making sure a result is found and comparing that
        result with the environment
        """
        pr_num = int(env['CIVET_PR_NUM'])
        if pr_num == 0:
            pr_num = None
        event_id = int(env['CIVET_EVENT_ID'])
        head_sha = env['CIVET_HEAD_SHA']

        for load_type in ['pr', 'event', 'commit']:
            reader = ResultsReader(TEST_DATABASE)
            if load_type == 'pr':
                if pr_num is None:
                    continue
                result = reader.getPRResults(pr_num)
            elif load_type == 'event':
                result = reader.getEventResults(event_id)
            elif load_type == 'commit':
                result = reader.getCommitResults(head_sha)
            self.assertIsNotNone(result)
            self.compareResults(result, env, base_sha, id)

    @unittest.skipUnless(HAS_STORE_AUTH, f"Skipping because store auth is not available")
    @unittest.skipUnless(HAS_READER_AUTH, f"Skipping because reader auth is not available")
    def testPR(self):
        """
        Test storing a pull request result and then loading
        it using the reader in all the ways it can be searched
        """
        run_test_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS)
        result = run_test_result.results

        base_sha, base_env = base_civet_env()
        pr_num = random_id()
        env = {
            'CIVET_EVENT_CAUSE': 'Pull request',
            'CIVET_PR_NUM': str(pr_num)
        }
        env.update(base_env)

        result_id, test_ids = CIVETStore().store(TEST_DATABASE, result, base_sha, env=env)
        self.assertIsNone(test_ids)

        try:
            # Test get[Event,PR,Commit]Results()
            self.runGetResultsTest(env, base_sha, result_id)

            # Get by test results with a PR option, from which the
            # first one should be the one from the PR
            test_reader = ResultsReader(TEST_DATABASE)
            test_results = test_reader.getTestResults(TEST_NAME.folder, TEST_NAME.name,
                                                      limit=1, pr_num=pr_num)
            self.assertEqual(len(test_results), 1)
            test_result = test_results[0]
            self.assertEqual(test_result.name, TEST_NAME)
            self.assertEqual(test_result.result_id, result_id)
            self.compareResults(test_result.results, env, base_sha, result_id)

        except:
            self.deleteDocuments(result_id)
            raise

        self.deleteDocuments(result_id)

    @unittest.skipUnless(HAS_STORE_AUTH, f"Skipping because store auth is not available")
    @unittest.skipUnless(HAS_READER_AUTH, f"Skipping because reader auth is not available")
    def testPush(self):
        """
        Test storing a push event result and then loading
        it using the reader in all the ways it can be searched
        """
        run_test_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS)
        result = run_test_result.results

        base_sha, base_env = base_civet_env()
        env = {
            'CIVET_EVENT_CAUSE': 'Push next',
            'CIVET_PR_NUM': '0'
        }
        env.update(base_env)

        result_id, test_ids = CIVETStore().store(TEST_DATABASE, result, base_sha, env=env)
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
            test_results = test_reader.getTestResults(TEST_NAME.folder, TEST_NAME.name, limit=50)
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

    @unittest.skipUnless(HAS_STORE_AUTH, f"Skipping because store auth is not available")
    @unittest.skipUnless(HAS_READER_AUTH, f"Skipping because reader auth is not available")
    def testSeparateTests(self):
        """
        Test storing an event where the tests are stored
        in separate documents from the main results
        """
        run_test_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS)
        result = run_test_result.results

        base_sha, base_env = base_civet_env()
        pr_num = random_id()
        env = {
            'CIVET_EVENT_CAUSE': 'Pull request',
            'CIVET_PR_NUM': str(pr_num)
        }
        env.update(base_env)
        event_sha = env['CIVET_HEAD_SHA']

        result_id, test_ids = CIVETStore().store(TEST_DATABASE, result, base_sha, env=env, max_result_size=1e-6)
        self.assertIsNotNone(test_ids)
        self.assertEqual(len(test_ids), 1)

        try:
            reader = ResultsReader(TEST_DATABASE)
            results = reader.getCommitResults(event_sha)
            self.assertIsNotNone(result)
            self.compareResults(results, env, base_sha, result_id)
            pr_test = results.get_test(TEST_NAME.folder, TEST_NAME.name)
            self.assertEqual(pr_test.id, test_ids[0])
        except:
            self.deleteDocuments(result_id, test_ids)
            raise

        self.deleteDocuments(result_id, test_ids)


if __name__ == '__main__':
    unittest.main()
