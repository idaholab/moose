# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test gold results for resultsstore."""

import json
import os
import unittest

import pytest
from TestHarness.resultsstore.reader import ResultsReader
from TestHarness.resultsstore.resultcollection import ResultsCollection
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.storedtestresult import StoredTestResult
from TestHarness.resultsstore.testdatafilters import TestDataFilter
from TestHarness.tests.resultsstore.common import (
    GOLD_DATABASE_NAME,
    GOLD_DATABASE_TEST_NAME,
    GOLD_RESULTS,
    FakeMongoClient,
    ResultsStoreTestCase,
)

# Authentication for the reader
READER_AUTH = ResultsReader.load_authentication()
HAS_READER_AUTH = READER_AUTH is not None

GOLD_STORED_RESULTS_FILE = os.path.join(
    os.path.dirname(__file__), "content", "gold_results.json"
)

# Set to true to update the gold files
UPDATE_GOLD_FILES = False


class TestGold(ResultsStoreTestCase):
    """Test golded results for resultsstore."""

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_READER_AUTH, "Reader auth unavailable")
    def test_gold_live(self):
        """Test StoredResult and StoredTestResult for each gold."""
        ids = [v.id for v in GOLD_RESULTS]

        # Load results
        with ResultsReader(GOLD_DATABASE_NAME, authentication=READER_AUTH) as ctx:
            reader = ctx.reader
            docs = reader._find_results({"_id": {"$in": ids}}, limit=None)
            self.assertEqual(len(docs), len(ids))

            # Build each gold result
            results = [reader._build_result(doc) for doc in docs]

            # Load target test from each result
            collection = ResultsCollection(results, ctx.reader.get_database)
            tests = collection.get_tests(GOLD_DATABASE_TEST_NAME, [TestDataFilter.ALL])
            self.assertEqual(len(tests), len(GOLD_RESULTS))

        # Serialize the data to store in a gold
        results_serialized = {str(v.id): v.serialize() for v in results}
        tests_serialized = {str(v.result.id): v.serialize() for v in tests}
        gold = {"results": results_serialized, "tests": tests_serialized}

        # Update the gold if requested
        if UPDATE_GOLD_FILES:
            with open(GOLD_STORED_RESULTS_FILE, "w") as f:
                json.dump(gold, f, indent=2)

        # Load gold and compare
        with open(GOLD_STORED_RESULTS_FILE, "r") as f:
            gold_loaded = json.load(f)
        self.assertEqual(gold, gold_loaded)

    def test_gold(self):
        """Build a StoredResult and StoredTestResult for each gold test."""
        # Load gold
        with open(GOLD_STORED_RESULTS_FILE, "r") as f:
            gold = json.load(f)
        gold_result_data = gold["results"]
        gold_test_data = gold["tests"]
        self.assertEqual(len(gold_result_data), len(GOLD_RESULTS))
        self.assertEqual(len(gold_test_data), len(GOLD_RESULTS))

        # Construct and test for each casde
        for gold_result in GOLD_RESULTS:
            result_id = str(gold_result.id)

            # Build StoredResult
            result_data = gold_result_data[result_id]
            data = StoredResult.deserialize(result_data)
            reader = ResultsReader("unused", FakeMongoClient())
            result = reader._build_result(data)

            # Check StoredResult
            self.assertEqual(result.id, gold_result.id)
            self.assertEqual(result.civet_version, gold_result.civet_version)
            self.assertEqual(result.event_sha, gold_result.event_sha)
            if gold_result.event_id:
                self.assertEqual(result.event_id, gold_result.event_id)

            # Build StoredTestResult
            test_data = gold_test_data[result_id]
            StoredTestResult.deserialize(test_data, result)
