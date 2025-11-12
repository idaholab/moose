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
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.tests.resultsstore.common import (
    GOLD_DATABASE_NAME,
    GOLD_RESULTS,
    FakeMongoClient,
    ResultsStoreTestCase,
)

# Authentication for the reader
READER_AUTH = ResultsReader.load_authentication()
HAS_READER_AUTH = READER_AUTH is not None

GOLD_STORED_RESULTS_FILE = os.path.join(
    os.path.dirname(__file__), "content", "gold_stored_results.json"
)

UPDATE_GOLD_FILES = False


class TestGold(ResultsStoreTestCase):
    """Test golded results for resultsstore."""

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_READER_AUTH, "Reader auth unavailable")
    def test_stored_results_live(self):
        """Test StoredResult for each gold."""
        ids = [v.id for v in GOLD_RESULTS]

        # Load from the database
        with ResultsReader(GOLD_DATABASE_NAME, authentication=READER_AUTH) as ctx:
            reader = ctx.reader
            docs = reader._find_results({"_id": {"$in": ids}}, limit=None)
            self.assertEqual(len(docs), len(ids))
        results = [reader._build_result(doc) for doc in docs]
        serialized = {str(v.id): v.serialize() for v in results}

        # Update the gold if requested
        if UPDATE_GOLD_FILES:
            with open(GOLD_STORED_RESULTS_FILE, "w") as f:
                json.dump(serialized, f, indent=2)

        # Load gold to compare
        with open(GOLD_STORED_RESULTS_FILE, "r") as f:
            gold = json.load(f)
        self.assertEqual(len(gold), len(ids))

        # Make sure the is no difference in golds
        for id in ids:
            id = str(id)
            self.assertEqual(serialized[id], gold[id])

    def test_stored_results(self):
        """Build a StoredResult for each gold."""
        ids = [v.id for v in GOLD_RESULTS]

        # Load gold
        with open(GOLD_STORED_RESULTS_FILE, "r") as f:
            gold = json.load(f)
        self.assertEqual(len(gold), len(ids))

        # Construct a StoredResult for each gold and compare
        for gold_result in GOLD_RESULTS:
            data = StoredResult.deserialize(gold[str(gold_result.id)])

            reader = ResultsReader("unused", FakeMongoClient())
            result = reader._build_result(data)

            self.assertEqual(result.id, gold_result.id)
            self.assertEqual(result.civet_version, gold_result.civet_version)
            self.assertEqual(result.event_sha, gold_result.event_sha)
            if gold_result.event_id:
                self.assertEqual(result.event_id, gold_result.event_id)
