#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from datetime import datetime
from mock import patch
from typing import Tuple
from dataclasses import dataclass
import os
import json

from pymongo import MongoClient
from bson.objectid import ObjectId

from TestHarness.resultsreader.reader import TestHarnessResultsReader
from TestHarness.resultsreader.results import TestHarnessResults, TestHarnessTestResult

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = TestHarnessResultsReader.hasEnvironmentAuthentication()

# Production database file for testing get_test_results
PROD_GET_TEST_RESULTS_GOLD_PATH = os.path.join(os.path.dirname(__file__), 'gold', 'resultsreader', 'prod_get_test_results.json')
# Production database name for testing real results
PROD_DATABASE_NAME = 'civet_tests_moose_performance'
# Arguments for using the production database for getting real results
PROD_FOLDER_NAME = 'simple_transient_diffusion'
PROD_TEST_NAME = 'test'

# Test database name for testing pull request results
TEST_DATABASE_NAME = 'civet_tests_moose_test_results'
# Arguments for using the production database for getting real results
TEST_FOLDER_NAME = 'tests/test_harness'
TEST_TEST_NAME = 'ok'

class FakeMongoClient(MongoClient):
    def __init__(self, *args, **kwargs):
        pass

    def list_database_names(self):
        return [PROD_DATABASE_NAME]

    def get_database(self, name):
        assert name == PROD_DATABASE_NAME

    def close(self):
        pass

class TestResultsReaderReader(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def buildProdGetTestResultsGold(self):
        """
        Helper for building the gold file for testing getTestResults(),
        using a live production database
        """
        @dataclass
        class GoldTest:
            event_sha: str | None
            event_id: str | None
            civet_version: int

        # Static set of test IDs (test entires in the database) that we
        # will always test with
        gold_tests = [GoldTest(event_sha='968f537a3c89ffb556fb7da18da28da52b592ee0',
                               event_id=None,
                               civet_version=0),
                      GoldTest(event_sha='45b8a536530388e7bb1ff563398b1e94f2d691fc',
                               event_id=None,
                               civet_version=1),
                      GoldTest(event_sha='3d48fa82c081e141fd86390dfb9edd1f11c984ca',
                               event_id=None,
                               civet_version=0),
                      # bump to civet_version=2
                      GoldTest(event_sha='1134c7e383972783be2ea702f2738ece79fe6a59',
                               event_id=None,
                               civet_version=2),
                      # bump to civet_version=3 (added event_id)
                      GoldTest(event_sha=None,
                               event_id=258481,
                               civet_version=3)]

        # This can be set to true once to overwrite the gold file
        rewrite_gold = False

        reader = TestHarnessResultsReader(PROD_DATABASE_NAME)

        # Load the results separately
        gold = {'results': {}, 'tests': {}}
        for gold_test in gold_tests:
            results = None
            if gold_test.event_sha:
                results = reader.getCommitResults(gold_test.event_sha)
            elif gold_test.event_id:
                results = reader.getEventResults(gold_test.event_id)
            self.assertIsNotNone(results)

            gold['results'][str(results.id)] = results.data

            test_results = results.get_test(PROD_FOLDER_NAME, PROD_TEST_NAME)
            self.assertIsNotNone(test_results)
            gold['tests'][str(test_results.id)] = test_results.data

            # Remove json_metadata as it's lengthy binary
            gold['tests'][str(test_results.id)]['tester']['json_metadata'] = {}

            # Checks
            self.assertEqual(results.civet_version, gold_test.civet_version)

        # Dump the values so that we can load them
        gold_dumped = json.dumps(gold, default=str, indent=2, sort_keys=True)

        # Rewrite the gold file if we set to do so
        if rewrite_gold:
            with open(PROD_GET_TEST_RESULTS_GOLD_PATH, 'w') as f:
                f.write(gold_dumped)

        return gold_dumped

    @staticmethod
    def getGetTestResultsGold() -> Tuple[list[dict], dict[str, dict]]:
        """
        Helper for getting the golded entries for the getTestResults() tests
        """
        with open(PROD_GET_TEST_RESULTS_GOLD_PATH, 'r') as f:
            gold = json.load(f)

        # Convert datetime strings to objects
        results = gold['results']
        for result in results.values():
            result['time'] = datetime.fromisoformat(result['time'])
            result['_id'] = ObjectId(result['_id'])
        tests = gold['tests']
        for test in tests.values():
            test['time'] = datetime.fromisoformat(test['time'])
            test['_id'] = ObjectId(test['_id'])

        return results, tests

    def testHasAuth(self):
        """
        Helper for checking if auth is available or not given an environment
        variable. This lets us within CIVET tests assert whether or not
        the authentication is available when we expect it to be so (or not).
        """
        if os.environ.get('TEST_RESULTSREADER_HAS_AUTH') is not None:
            self.assertTrue(HAS_AUTH)
        if os.environ.get('TEST_RESULTSREADER_MISSING_AUTH') is not None:
            self.assertFalse(HAS_AUTH)

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testGetTestResultsGold(self):
        """
        Tests that generating the gold file (using static test entries) using
        the live server gets us the same gold file that we currently have
        """
        new_gold = self.buildProdGetTestResultsGold()

        with open(PROD_GET_TEST_RESULTS_GOLD_PATH, 'r') as f:
            gold = json.load(f)

        self.assertEqual(gold, json.loads(new_gold))

    def _testGetTestResults(self, reader: TestHarnessResultsReader, **kwargs) -> list[TestHarnessTestResult]:
        """
        Helper for testing getTestResults(), regardless of if
        the data was produced from a gold file or live
        """
        results = reader.getTestResults(PROD_FOLDER_NAME, PROD_TEST_NAME, **kwargs)

        for result in results:
            self.assertIsInstance(result, TestHarnessTestResult)

            # Test that result is the correct one
            test_harness_results = result.results
            self.assertEqual(test_harness_results, result.results)
            self.assertEqual(PROD_FOLDER_NAME, result.folder_name)
            self.assertEqual(PROD_TEST_NAME, result.test_name)
            self.assertIsInstance(test_harness_results, TestHarnessResults)

            # Test basic state for results header
            self.assertRegex(test_harness_results.civet_job_url, r'civet.inl.gov/job/\d+')
            self.assertEqual(test_harness_results.event_cause, 'push')

            # Event ID as of civet version 3
            if result.results.civet_version > 2:
                self.assertTrue(isinstance(test_harness_results.event_id, int))
            else:
                self.assertIsNone(test_harness_results.event_id)

            # Should have CIVET state
            self.assertEqual(result.event_cause, 'push')
            self.assertIsNone(result.pr_num)

            # Test that basic entries have values
            self.assertGreater(result.run_time, 0)
            self.assertIsNotNone(result.hpc_queued_time)

        return results

    @patch.object(TestHarnessResultsReader, '_findResults')
    @patch.object(TestHarnessResults, '_find_test_data')
    def testGetTestResults(self, patch_find_test_data, patch_find_results):
        """
        Tests calling getTestResults() using mocked returns from mongodb
        """
        # Get the gold entry so that we can mimic calling pymongo
        gold_results, gold_tests = self.getGetTestResultsGold()
        # Mock getting the test results from mongodb
        def get_test_data(id):
            return gold_tests[id]
        patch_find_test_data.side_effect = get_test_data
        # Mock getting the results from mongodb
        def get_results_data(*args, **kwargs):
            filter = args[0]
            if '_id' not in filter:
                return list(gold_results.values())
            return []
        patch_find_results.side_effect = get_results_data

        reader = TestHarnessResultsReader(PROD_DATABASE_NAME, FakeMongoClient())
        test_results = self._testGetTestResults(reader)
        self.assertEqual(len(test_results), len(gold_tests))

        for test_result in test_results:
            self.assertNotIn(test_result.id, gold_tests)

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testGetTestResultsLive(self):
        """
        Tests calling getTestResults() using the real server, if available
        """
        reader = TestHarnessResultsReader(PROD_DATABASE_NAME)
        limit = 10
        results = self._testGetTestResults(reader, limit=limit)
        self.assertEqual(len(results), limit)

    @unittest.skipIf(HAS_AUTH, f"Skipping because authentication is available")
    def testMissingClient(self):
        """
        Tests creating the TestHarnessResultsReader without a client/auth
        """
        with self.assertRaisesRegex(ValueError, 'Must specify'):
            TestHarnessResultsReader('unused')

    def testInvalidClient(self):
        """
        Tests creating the TestHarnessResultsReader without a valid client/auth
        """
        with self.assertRaisesRegex(TypeError, "Invalid type for 'client'"):
            TestHarnessResultsReader('unused', 'bad_client')

    def testMissingDatabase(self):
        """
        Tests creating the TestHarnessResultsReader with a database that isn't found,
        with a mocked database
        """
        class BadDatabaseClient(FakeMongoClient):
            def list_database_names(self):
                return ['foo']

        with self.assertRaisesRegex(ValueError, f'Database {PROD_DATABASE_NAME} not found'):
            TestHarnessResultsReader(PROD_DATABASE_NAME, BadDatabaseClient())

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testMissingDatabaseLive(self):
        """
        Tests creating the TestHarnessResultsReader with a database that isn't found
        """
        name = 'foo1234'
        with self.assertRaisesRegex(ValueError, f'Database {name} not found'):
            TestHarnessResultsReader(name)

    @unittest.skipUnless(os.environ.get('TEST_RESULTSREADER_READER'), f"Skipping because TEST_RESULTSREADER_READER not set")
    def testGetTestsWithPRLive(self):
        """
        Tests getTestsResults when we have PR/event data in the test database.

        This is explicitly tested with CIVET, where a step before this
        or a dependency is able to contribute to TEST_DATABASE_NAME
        """
        self.assertTrue(HAS_AUTH)

        head_sha = os.environ.get('CIVET_HEAD_SHA')
        self.assertIsNotNone(head_sha)
        self.assertEqual(len(head_sha), 40)

        event_cause = os.environ.get('CIVET_EVENT_CAUSE')
        self.assertIsNotNone(event_cause)
        is_pr = event_cause.startswith('Pull')
        pr_num = None

        event_id = os.environ.get('CIVET_EVENT_ID')
        self.assertIsNotNone(event_id)
        event_id = int(event_id)

        job_id = os.environ.get('CIVET_JOB_ID')
        self.assertIsNotNone(job_id)
        job_id = int(job_id)

        if is_pr:
            pr_num = os.environ.get('CIVET_PR_NUM')
            self.assertIsNotNone('CIVET_PR_NUM')
            pr_num = int(pr_num)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
        results = reader.getTestResults(TEST_FOLDER_NAME, TEST_TEST_NAME, pr_num=pr_num)
        self.assertGreater(len(results), 0)

        result = results[0]
        self.assertEqual(result.event_id, event_id)
        self.assertEqual(result.folder_name, TEST_FOLDER_NAME)
        self.assertEqual(result.test_name, TEST_TEST_NAME)
        if is_pr:
            self.assertEqual(result.pr_num, pr_num)
            self.assertEqual(result.event_sha, head_sha)
            self.assertEqual(result.event_cause, 'pr')
        else:
            self.assertIsNone(result.pr_num)
            self.assertEqual(result.event_cause, 'push')

            event_results = [r for r in results if r.event_sha == head_sha]
            self.assertEqual(len(event_results), 1)

            result = event_results[0]

        start_index = 0
        if is_pr:
            start_index = 1
        for result in results[start_index:]:
            self.assertNotEqual(result.event_cause, 'pr')

    @unittest.skipUnless(os.environ.get('TEST_RESULTSREADER_READER'), f"Skipping because TEST_RESULTSREADER_READER not set")
    def testGetEventResultsLive(self):
        """
        Tests getEventResults when we have PR/event data in the test database.

        This is explicitly tested with CIVET, where a step before this
        or a dependency is able to contribute to TEST_DATABASE_NAME
        """
        self.assertTrue(HAS_AUTH)

        event_id = os.environ.get('CIVET_EVENT_ID')
        self.assertIsNotNone(event_id)
        event_id = int(event_id)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id)
        self.assertIsNotNone(results)
        self.assertEqual(results.event_id, event_id)

        self.assertEqual(len(results.test_names), 1)
        test_results = results.get_test(TEST_FOLDER_NAME, TEST_TEST_NAME)
        self.assertIsNotNone(test_results)
        self.assertEqual(test_results.folder_name, TEST_FOLDER_NAME)
        self.assertEqual(test_results.test_name, TEST_TEST_NAME)

    @unittest.skipUnless(os.environ.get('TEST_RESULTSREADER_READER'), f"Skipping because TEST_RESULTSREADER_READER not set")
    def testGetCommitResultsLive(self):
        """
        Tests getCommitResults when we have PR/event data in the test database.

        This is explicitly tested with CIVET, where a step before this
        or a dependency is able to contribute to TEST_DATABASE_NAME
        """
        self.assertTrue(HAS_AUTH)

        head_sha = os.environ.get('CIVET_HEAD_SHA')
        self.assertIsNotNone(head_sha)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
        results = reader.getCommitResults(head_sha)
        self.assertIsNotNone(results)
        self.assertEqual(results.event_sha, head_sha)

        self.assertEqual(len(results.test_names), 1)
        test_results = results.get_test(TEST_FOLDER_NAME, TEST_TEST_NAME)
        self.assertIsNotNone(test_results)
        self.assertEqual(test_results.folder_name, TEST_FOLDER_NAME)
        self.assertEqual(test_results.test_name, TEST_TEST_NAME)

if __name__ == '__main__':
    unittest.main()
