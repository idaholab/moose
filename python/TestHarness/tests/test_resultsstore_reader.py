#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from mock import patch
from dataclasses import dataclass
from copy import deepcopy
from collections import OrderedDict
import os
import json

from pymongo import MongoClient
from bson.objectid import ObjectId

from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.storedresults import StoredResult, StoredTestResult, TestName

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = ResultsReader.hasEnvironmentAuthentication()

# Production database file for testing get_test_results
PROD_GET_TEST_RESULTS_GOLD_PATH = os.path.join(os.path.dirname(__file__), 'gold', 'resultsstore', 'prod_get_test_results.json')
# Production database name for testing real results
PROD_DATABASE_NAME = 'civet_tests_moose_performance'
# The name of the test to load from the production database
PROD_TEST_NAME = TestName('simple_transient_diffusion', 'test')

# Test database name for testing pull request results
TEST_DATABASE_NAME = 'civet_tests_moose_test_results'
# The name of the test to load from the test database
TEST_TEST_NAME = TestName('tests/test_harness', 'ok')

class FakeMongoClient(MongoClient):
    def __init__(self, *args, **kwargs):
        pass

    def list_database_names(self):
        return [PROD_DATABASE_NAME]

    def get_database(self, name):
        assert name == PROD_DATABASE_NAME

    def close(self):
        pass

class TestResultsReader(unittest.TestCase):
    """
    Tests the ResultsReader object, which loads data
    from the database stored by CIVETStore.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def buildProdGetTestResultsGold(self):
        """
        Builds a gold file for testing getTestResults(), using data
        from a live production database
        """
        @dataclass
        class GoldTest:
            event_sha: str | None
            event_id: str | None
            civet_version: int

        # Static set of results from which to build the gold file
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
                               civet_version=3),
                      # bump to civet_version=4 (remove indices from tests)
                      GoldTest(event_sha=None,
                               event_id=259309,
                               civet_version=4)]

        # This can be set to true once to overwrite the gold file
        rewrite_gold = False

        reader = ResultsReader(PROD_DATABASE_NAME)

        # Load each result, where gold is indexed from
        # result id -> result data
        gold = {}
        for gold_test in gold_tests:
            results = None
            if gold_test.event_sha:
                results = reader.getCommitResults(gold_test.event_sha)
            elif gold_test.event_id:
                results = reader.getEventResults(gold_test.event_id)
            self.assertIsNotNone(results)
            self.assertTrue(results.has_test(PROD_TEST_NAME.folder, PROD_TEST_NAME.name))

            gold[str(results.id)] = results.serialize()

            # Checks
            self.assertEqual(results.civet_version, gold_test.civet_version)

        # Dump the values so that we can load them
        gold_dumped = json.dumps(gold, indent=2, sort_keys=True)

        # Rewrite the gold file if we set to do so
        if rewrite_gold:
            with open(PROD_GET_TEST_RESULTS_GOLD_PATH, 'w') as f:
                f.write(gold_dumped)

        return gold_dumped

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

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testGetTestResultsGold(self):
        """
        Tests that generating the gold file (using static test entries) using
        the live server gets us the same gold file that we currently have,
        which should not differ
        """
        new_gold = self.buildProdGetTestResultsGold()

        with open(PROD_GET_TEST_RESULTS_GOLD_PATH, 'r') as f:
            gold = json.load(f)

        self.assertEqual(gold, json.loads(new_gold))

    def _testGetTestResults(self, reader: ResultsReader, **kwargs) -> list[StoredTestResult]:
        """
        Helper for testing getTestResults(), regardless of if
        the data was produced from a gold file or live
        """
        results = reader.getTestResults(PROD_TEST_NAME.folder, PROD_TEST_NAME.name, **kwargs)

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
        Helper for deserializing the gold data into a dict
        of result ID -> result data, sorted by ID
        (latest event first)
        """
        with open(PROD_GET_TEST_RESULTS_GOLD_PATH, 'r') as f:
            gold = json.load(f)
        gold = {ObjectId(id): StoredResult.deserialize(entry) for id, entry in gold.items()}
        return dict(OrderedDict(sorted(gold.items(), key=lambda x: x[0], reverse=True)))

    @patch.object(ResultsReader, '_findResults')
    @patch.object(StoredResult, '_find_test_data')
    def testGetTestResultsTestsSeparate(self, patch_find_test_data, patch_find_results):
        """
        Tests calling getTestResults() using mocked returns from mongodb when
        the tests are not stored within the result data entry, which requires
        queries within StoredResult to get the test data
        """
        gold = self.deserializeGold()

        tests: dict[ObjectId, dict] = {}
        # Remove the tests entries and store them as IDs instead
        for result_entry in gold.values():
            for folder_entry in result_entry['tests'].values():
                for test_name in list(folder_entry['tests'].keys()):
                    test_entry = folder_entry['tests'][test_name]
                    tests[test_entry['_id']] = deepcopy(test_entry)
                    folder_entry['tests'][test_name] = test_entry['_id']

        # Mock getting a test from mongodb
        def find_test_data(id):
            return tests[id]
        patch_find_test_data.side_effect = find_test_data
        # Mock getting the results from mongodb
        def get_results_data(*args, **kwargs):
            filter = args[0]
            if '_id' not in filter:
                return list(gold.values())
            return []
        patch_find_results.side_effect = get_results_data

        reader = ResultsReader(PROD_DATABASE_NAME, FakeMongoClient())
        test_results = self._testGetTestResults(reader)

        # Make sure we have a test entry for each gold test, in the right order
        self.assertEqual(len(test_results), len(gold))
        for i, entry in enumerate(gold):
            self.assertEqual(entry, test_results[i].result_id)

    @patch.object(ResultsReader, '_findResults')
    def testGetTestResultsTestsIncluded(self, patch_find_results):
        """
        Tests calling getTestResults() using mocked returns from mongodb when
        the tests are stored within the results entry, thus there are no
        database calls from within the StoredResult
        """
        gold = self.deserializeGold()

        # Mock getting the results from mongodb
        def get_results_data(*args, **kwargs):
            filter = args[0]
            if '_id' not in filter:
                return list(gold.values())
            return []
        patch_find_results.side_effect = get_results_data

        reader = ResultsReader(PROD_DATABASE_NAME, FakeMongoClient())
        test_results = self._testGetTestResults(reader)

        # Make sure we have a test entry for each gold test, in the right order
        self.assertEqual(len(test_results), len(gold))
        for i, entry in enumerate(gold):
            self.assertEqual(entry, test_results[i].result_id)

    @patch.object(ResultsReader, '_findResults')
    def testGetTestResultsWithLimit(self, patch_find_results):
        """
        Tests calling getTestResults() with a limit
        """
        gold = self.deserializeGold()

        # Mock getting the results from mongodb
        def get_results_data(*args, **kwargs):
            filter = args[0]
            if '_id' not in filter:
                return list(gold.values())
            return []
        patch_find_results.side_effect = get_results_data

        reader = ResultsReader(PROD_DATABASE_NAME, FakeMongoClient())
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
        """
        Tests calling getTestResults() using the real server, if available
        """
        reader = ResultsReader(PROD_DATABASE_NAME)

        for limit in [1, 2, 3]:
            results = self._testGetTestResults(reader, limit=limit)
            self.assertEqual(len(results), limit)
            self.assertEqual(len(reader._latest_push_results), limit)

    @unittest.skipIf(HAS_AUTH, f"Skipping because authentication is available")
    def testMissingClient(self):
        """
        Tests creating the ResultsReader without a client/auth
        """
        with self.assertRaisesRegex(ValueError, 'Must specify'):
            ResultsReader('unused')

    def testInvalidClient(self):
        """
        Tests creating the ResultsReader without a valid client/auth
        """
        with self.assertRaisesRegex(TypeError, "Invalid type for 'client'"):
            ResultsReader('unused', 'bad_client')

    def testMissingDatabase(self):
        """
        Tests creating the ResultsReader with a database that isn't found,
        with a mocked database
        """
        class BadDatabaseClient(FakeMongoClient):
            def list_database_names(self):
                return ['foo']

        with self.assertRaisesRegex(ValueError, f'Database {PROD_DATABASE_NAME} not found'):
            ResultsReader(PROD_DATABASE_NAME, BadDatabaseClient())

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testMissingDatabaseLive(self):
        """
        Tests creating the ResultsReader with a database that isn't found
        """
        name = 'foo1234'
        with self.assertRaisesRegex(ValueError, f'Database {name} not found'):
            ResultsReader(name)

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

        reader = ResultsReader(TEST_DATABASE_NAME)
        results = reader.getTestResults(TEST_TEST_NAME.folder, TEST_TEST_NAME.name, pr_num=pr_num)
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
            self.assertEqual(result.event_cause, 'pr')
        # If we're not on a PR, the latest data
        # should be a push event
        else:
            self.assertIsNone(result.pr_num)
            self.assertEqual(result.event_cause, 'push')

            event_results = [r for r in results if r.event_sha == head_sha]
            self.assertEqual(len(event_results), 1)

        # Make sure that every event is a push event, except
        # if we're on a PR in which all but the first should
        # be a push event and that all of the events should
        # be in decreasing id (newer to older)
        start_index = 1 if is_pr else 0
        last_id = None
        for result in results[start_index:]:
            self.assertNotEqual(result.event_cause, 'pr')
            if last_id is not None:
                self.assertGreater(last_id, result.id)
            last_id = result.id

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

        reader = ResultsReader(TEST_DATABASE_NAME)
        results = reader.getEventResults(event_id)
        self.assertIsNotNone(results)
        self.assertEqual(results.event_id, event_id)

        self.assertEqual(len(results.test_names), 1)
        test_results = results.get_test(TEST_TEST_NAME.folder, TEST_TEST_NAME.name)
        self.assertIsNotNone(test_results)
        self.assertEqual(test_results.name, TEST_TEST_NAME)

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

        reader = ResultsReader(TEST_DATABASE_NAME)
        results = reader.getCommitResults(head_sha)
        self.assertIsNotNone(results)
        self.assertEqual(results.event_sha, head_sha)

        self.assertEqual(len(results.test_names), 1)
        test_results = results.get_test(TEST_TEST_NAME.folder, TEST_TEST_NAME.name)
        self.assertIsNotNone(test_results)
        self.assertEqual(test_results.name, TEST_TEST_NAME)

if __name__ == '__main__':
    unittest.main()
