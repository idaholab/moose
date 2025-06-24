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
import os
import json

from pymongo import MongoClient
from bson.objectid import ObjectId

from TestHarness.resultsreader.reader import TestHarnessResultsReader
from TestHarness.resultsreader.results import TestHarnessResults, TestHarnessTestResult

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = TestHarnessResultsReader.hasEnvironmentAuthentication()

# Gold file for testing get_test_results
GET_TEST_RESULTS_GOLD_PATH = os.path.join(os.path.dirname(__file__), 'gold', 'resultsreader', 'get_test_results.json')

DATABASE_NAME = 'civet_tests_moose_performance'
GET_TEST_RESULTS_ARGS = {'folder_name': 'simple_transient_diffusion',
                         'test_name': 'test',
                         'limit': 2}

class FakeMongoClient(MongoClient):
    def __init__(self, *args, **kwargs):
        pass

    def list_database_names(self):
        return [DATABASE_NAME]

    def get_database(self, name):
        assert name == DATABASE_NAME

    def close(self):
        pass

class TestResultsReaderReader(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def buildGetTestResultsGold(self):
        """
        Helper for building the gold file for testing getTestResults(),
        using the live database
        """
        # Static set of test IDs (test entires in the database) that we
        # will always test with. These can be statically updated in the future
        # by uncommenting the lines a btit below
        gold_test_ids = ['685b0fdf4110325560e2cc2f', '6857a572bbcb03d9dccfb1a7']
        # This can be set to true once to overwrite the gold file
        rewrite_gold = False

        reader = TestHarnessResultsReader(DATABASE_NAME)

        # Uncomment this to update the latest test IDs for regolding
        # (should staticly set the values above from this if needed)
        # last_ids = [v['_id'] for v in reader._getTestsEntry(**GET_TEST_RESULTS_ARGS)]
        # print(last_ids)

        # Get only the specific tests that we've golded on
        id_filter = {"$or": [{"_id": ObjectId(id)} for id in gold_test_ids]}
        tests = reader._getTestsEntry(**GET_TEST_RESULTS_ARGS, filter=id_filter)
        self.assertEqual(len(tests), len(gold_test_ids))

        # Load the results separately
        results = {}
        for test in tests:
            result = reader._getResultsEntry(test['result_id'])
            results[str(result['_id'])] = result

        # Dump the values so that we can load them
        values = {'tests': tests, 'results': results}
        values_dumped = json.dumps(values, default=str)

        # Rewrite the gold file if we set to do so
        if rewrite_gold:
            with open(GET_TEST_RESULTS_GOLD_PATH, 'w') as f:
                f.write(values_dumped)

        return values_dumped

    @staticmethod
    def getGetTestResultsGold() -> Tuple[list[dict], dict[str, dict]]:
        """
        Helper for getting the golded entries for the getTestResults() tests
        """
        with open(GET_TEST_RESULTS_GOLD_PATH, 'r') as f:
            gold = json.load(f)

        # Convert datetime strings to objects
        results = gold['results']
        for result in results.values():
            result['time'] = datetime.fromisoformat(result['time'])
        tests = gold['tests']
        for test in tests:
            test['time'] = datetime.fromisoformat(test['time'])

        return results, tests

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testGetTestResultsGold(self):
        """
        Tests that generating the gold file (using static test entries) using
        the live server gets us the same gold file that we currently have
        """
        new_gold = self.buildGetTestResultsGold()

        with open(GET_TEST_RESULTS_GOLD_PATH, 'r') as f:
            gold = json.load(f)

        self.assertEqual(gold, json.loads(new_gold))

    def _testGetTestResults(self, reader: TestHarnessResultsReader) -> list[TestHarnessTestResult]:
        """
        Helper for testing getTestResults(), regardless of if
        the data was produced from a gold file or live
        """
        results = reader.getTestResults(**GET_TEST_RESULTS_ARGS)
        self.assertEqual(len(results), 2)

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
            return gold_results[id]
        patch_get_results_entry.side_effect = get_results_entry
        # Mock getting the tests from mongodb
        patch_get_tests_entry.return_value = gold_tests

        reader = TestHarnessResultsReader(DATABASE_NAME, FakeMongoClient())
        self._testGetTestResults(reader)

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testGetTestResultsLive(self):
        """
        Tests calling getTestResults() using the real server, if available
        """
        reader = TestHarnessResultsReader(DATABASE_NAME)
        self._testGetTestResults(reader)

    @unittest.skipIf(HAS_AUTH, f"Skipping because authentication is not available")
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

        with self.assertRaisesRegex(ValueError, f'Database {DATABASE_NAME} not found'):
            TestHarnessResultsReader(DATABASE_NAME, BadDatabaseClient())

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
        reader = TestHarnessResultsReader(DATABASE_NAME)
        id = '1234'
        with self.assertRaisesRegex(KeyError, f'No {DATABASE_NAME}.results entry with _id={id}'):
            reader._getResultsEntry(id)

if __name__ == '__main__':
    unittest.main()
