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
PROD_GET_TEST_RESULTS_ARGS = {'folder_name': 'simple_transient_diffusion',
                              'test_name': 'test'}

# Test database name for testing pull request results
TEST_DATABASE_NAME = 'civet_tests_moose_test_results'
# Arguments for using the production database for getting real results
TEST_GET_TEST_RESULTS_ARGS = {'folder_name': 'tests/test_harness',
                              'test_name': 'ok'}

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
            id: str
            civet_version: int

        # Static set of test IDs (test entires in the database) that we
        # will always test with
        gold_tests = [GoldTest(id='685b0fdf4110325560e2cc2f', civet_version=None),
                      GoldTest(id='6857a572bbcb03d9dccfb1a7', civet_version=None),
                      GoldTest(id='685c623b4022db39df9590c3', civet_version=None),
                      # bump to civet_version=2
                      GoldTest(id='6865744be52cb57c4742666d', civet_version=2)]

        # This can be set to true once to overwrite the gold file
        rewrite_gold = False

        reader = TestHarnessResultsReader(PROD_DATABASE_NAME)

        # Get only the specific tests that we've golded on
        id_filter = {"$or": [{"_id": ObjectId(test.id)} for test in gold_tests]}
        tests = reader._getTestsEntry(**PROD_GET_TEST_RESULTS_ARGS, filter=id_filter)
        self.assertEqual(len(tests), len(gold_tests))

        # Load the results separately
        results = {}
        for gold_test in gold_tests:
            test_filter = [v for v in tests if v['_id'] == ObjectId(gold_test.id)]
            self.assertEqual(len(test_filter), 1)
            test = test_filter[0]
            result = reader._getResultsEntry(test['result_id'])
            results[str(result['_id'])] = result

            # Remove json_metadata as it's lengthy binary
            test['tester']['json_metadata'] = {}

            # Checks
            self.assertEqual(test.get('civet_version'), gold_test.civet_version)

        # Dump the values so that we can load them
        values = {'tests': tests, 'results': results}
        values_dumped = json.dumps(values, default=str, indent=2, sort_keys=True)

        # Rewrite the gold file if we set to do so
        if rewrite_gold:
            with open(PROD_GET_TEST_RESULTS_GOLD_PATH, 'w') as f:
                f.write(values_dumped)

        return values_dumped

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
        tests = gold['tests']
        for test in tests:
            test['time'] = datetime.fromisoformat(test['time'])

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
        results = reader.getTestResults(**PROD_GET_TEST_RESULTS_ARGS, **kwargs)

        for result in results:
            self.assertIsInstance(result, TestHarnessTestResult)

            # Test that result is the correct one
            test_harness_results = result.results
            self.assertEqual(test_harness_results.data['_id'], result.data['result_id'])
            self.assertIsInstance(test_harness_results, TestHarnessResults)

            # Test basic state for results header
            self.assertRegex(test_harness_results.civet_job_url, r'civet.inl.gov/job/\d+')
            self.assertEqual(test_harness_results.event_cause, 'push')

            # Should have CIVET state
            self.assertEqual(result.event_cause, 'push')
            self.assertIsNone(result.pr_num)

            # Test that basic entries have values
            self.assertGreater(result.run_time, 0)
            self.assertIsNotNone(result.hpc_queued_time)

        return results

    @patch.object(TestHarnessResultsReader, '_getTestsEntry')
    @patch.object(TestHarnessResultsReader, '_getResultsEntry')
    def testGetTestResults(self, patch_get_results_entry, patch_get_tests_entry):
        """
        Tests calling getTestResults() using mocked returns from mongodb
        """
        # Get the gold entry so that we can mimic calling pymongo
        gold_results, gold_tests = self.getGetTestResultsGold()
        # Mock getting the results from mongodb
        def get_results_entry(id):
            return gold_results[str(id)]
        patch_get_results_entry.side_effect = get_results_entry
        # Mock getting the tests from mongodb
        patch_get_tests_entry.return_value = gold_tests

        reader = TestHarnessResultsReader(PROD_DATABASE_NAME, FakeMongoClient())
        results = self._testGetTestResults(reader)
        self.assertEqual(len(results), len(gold_tests))

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

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testMissingResultsEntryLive(self):
        """
        Tests an exception being thrown from _getResultsEntry() with an invalid id
        """
        reader = TestHarnessResultsReader(PROD_DATABASE_NAME)
        id = ObjectId('400b0fdf4110325560e2cc2f')
        with self.assertRaisesRegex(KeyError, f'No {PROD_DATABASE_NAME}.results entry with _id={id}'):
            reader._getResultsEntry(id)

    @unittest.skipUnless(os.environ.get('TEST_RESULTSREADER_READER'), f"Skipping because TEST_RESULTSREADER_READER not set")
    def testGetTestsWithPRLive(self):
        """
        Tests getTestsResults when we have PR data.

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

        job_id = os.environ.get('CIVET_JOB_ID')
        self.assertIsNotNone(job_id)
        job_id = int(job_id)

        if is_pr:
            pr_num = os.environ.get('CIVET_PR_NUM')
            self.assertIsNotNone('CIVET_PR_NUM')
            pr_num = int(pr_num)

        reader = TestHarnessResultsReader(TEST_DATABASE_NAME)
        results = reader.getTestResults(**TEST_GET_TEST_RESULTS_ARGS, pr_num=pr_num)
        self.assertGreater(len(results), 0)

        if is_pr:
            result = results[0]
            self.assertEqual(result.pr_num, pr_num)
            self.assertEqual(result.event_sha, head_sha)
            self.assertEqual(result.event_cause, 'pr')
        else:
            self.assertIsNone(results[0].pr_num)
            self.assertEqual(results[0].event_cause, 'push')

            event_results = [r for r in results if r.event_sha == head_sha]
            self.assertEqual(len(event_results), 1)

            result = event_results[0]

        self.assertEqual(result.results.civet_job_id, job_id)

        start_index = 0
        if is_pr:
            start_index = 1
        for result in results[start_index:]:
            self.assertNotEqual(result.event_cause, 'pr')

if __name__ == '__main__':
    unittest.main()
