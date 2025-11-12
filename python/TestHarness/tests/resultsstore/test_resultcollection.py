# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.resultcollection.ResultCollection."""

from TestHarness.resultsstore.resultcollection import ResultCollection
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.tests.resultsstore.common import (
    FakeMongoDatabase,
    ResultsStoreTestCase,
)


class TestResultCollection(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.resultcollection.ResultCollection."""

    def test_init(self):
        """Test __init__() and basic properties/getters."""
        results = [StoredResult(self.get_result_data())]

        database = FakeMongoDatabase()

        def database_getter():
            return database

        collection = ResultCollection(results, database_getter)

        self.assertEqual(collection._results, results)
        self.assertEqual(collection._database_getter, database_getter)

        self.assertEqual(collection.results, results)
        self.assertEqual(collection.result_ids, [v.id for v in results])
        self.assertEqual(id(collection.get_database()), id(database))
