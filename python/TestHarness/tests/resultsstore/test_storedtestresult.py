# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.storedtestresult.StoredTestResult."""

import json
from copy import deepcopy
from dataclasses import asdict
from datetime import datetime
from typing import Optional, Tuple

from bson.objectid import ObjectId
from mock import patch
from TestHarness.resultsstore.civetstore import CIVETStore
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.resultsstore.storedtestresult import (
    StoredTestResult,
)
from TestHarness.resultsstore.testdatafilters import TestDataFilter
from TestHarness.resultsstore.utils import (
    ResultsTestIterator,
    compress_dict,
    results_test_iterator,
)
from TestHarness.tests.resultsstore.common import ResultsStoreTestCase
from TestHarness.tests.resultsstore.test_civetstore import build_civet_env
from TestHarness.tests.resultsstore.test_storedresult import build_stored_result

# ID for the tested built StoredResult object
RESULT_ID = ObjectId()
# A dummy environment from CIVET to build a dummy header
BASE_SHA, CIVET_ENV = build_civet_env()
# A dummy header (entries added to the result entry)
STORE_HEADER = CIVETStore.build_header(BASE_SHA, CIVET_ENV)
# Dummy value for a test's ['timing']['hpc_queued_time']
HPC_QUEUED_TIME = 1.234


class TestResultsStoredResults(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.storedtestresult.StoredTestResult."""

    def build_stored_test_result(
        self,
        test: ResultsTestIterator,
        stored_result: StoredResult,
        test_filter: TestDataFilter = TestDataFilter.ALL,
        separate_test: bool = False,
        update_data: Optional[dict] = None,
    ) -> StoredTestResult:
        """
        Build a StoredTestResult for testing.

        Arguments:
        ---------
        test : ResultsTestIterator
            The test to build the result from.
        stored_result : StoredResult
            The parent result object.

        Optional arguments:
        ------------------
        test_filter : TestDataFilter
            The filter to use for the test.
        separate_test : bool
            Whether or not to store the test separately.
        update_data : Optional[dict]
            Extra data to add to each test.

        """
        has_all = test_filter == TestDataFilter.ALL
        test_filters = [test_filter]
        data = deepcopy(test.value)

        # Compress JSON metadata, if any
        json_metadata = data["tester"].get("json_metadata")
        if json_metadata:
            data["tester"]["json_metadata"] = {
                k: compress_dict(v) for k, v in json_metadata.items()
            }

        # Remove data not in the filter
        if not has_all:
            for k in list(data.keys()):
                if test_filter.value != k:
                    del data[k]

        # Store info for separate tests if appropriate
        id = None
        if separate_test:
            id = ObjectId()
            data["_id"] = id
            data["result_id"] = stored_result.id

        # Update data if requested
        if update_data is not None:
            data.update(update_data)

        stored_test_result = StoredTestResult(
            data, test.name, stored_result, test_filters
        )

        # Test members
        self.assertEqual(stored_test_result._data, data)
        self.assertEqual(stored_test_result._name, test.name)
        self.assertEqual(stored_test_result._result, stored_result)
        self.assertEqual(stored_test_result._data_filters, test_filters)

        # Test basic properties
        self.assertEqual(stored_test_result.data, data)
        self.assertEqual(stored_test_result.result, stored_result)
        self.assertEqual(stored_test_result.result_id, stored_result.id)
        self.assertEqual(stored_test_result.name, test.name)
        self.assertEqual(stored_test_result.folder_name, test.name.folder)
        self.assertEqual(stored_test_result.test_name, test.name.name)

        return stored_test_result

    def get_stored_test_results(
        self,
        test_filter: TestDataFilter = TestDataFilter.ALL,
        separate_test: bool = False,
        check: bool = True,
        update_data: Optional[dict] = None,
    ) -> list[Tuple[StoredTestResult, dict]]:
        """
        Build StoredTestResult objects for testing.

        Optional arguments:
        ------------------
        test_filter : TestDataFilter
            The filter to use for the test.
        separate_test : bool
            Whether or not to store the test separately.
        check : bool
            Value of check to pass to the StoredResult.
        update_data : Optional[dict]
            Update test data with this dict, if any.
        """
        result = self.get_testharness_result("--capture-perf-graph")
        stored_result = build_stored_result(result, check=check)

        return [
            (
                self.build_stored_test_result(
                    test,
                    stored_result,
                    test_filter=test_filter,
                    separate_test=separate_test,
                    update_data=update_data,
                ),
                test.value,
            )
            for test in results_test_iterator(result)
        ]

    def get_stored_test_result(
        self,
        test_filter: TestDataFilter = TestDataFilter.ALL,
        separate_test: bool = False,
        check: bool = True,
    ) -> Tuple[StoredTestResult, dict]:
        """Get a built StoredTestResult and original data for testing."""
        result = self.get_testharness_result("--capture-perf-graph")
        stored_result = build_stored_result(result, check=check)
        test = next(results_test_iterator(result))
        return (
            self.build_stored_test_result(
                test,
                stored_result,
                test_filter=test_filter,
                separate_test=separate_test,
            ),
            test.value,
        )

    def test_not_separate_tests(self):
        """Test building without tests stored separately."""
        test, _ = self.get_stored_test_result(TestDataFilter.ALL, separate_test=False)
        self.assertIsNone(test.id)

    def test_separate_tests(self):
        """Test building with tests stored separately."""
        test, _ = self.get_stored_test_result(TestDataFilter.ALL, separate_test=True)
        self.assertIsInstance(test.id, ObjectId)

    def test_check(self):
        """Test building with checking in the parent result."""
        with patch(
            "TestHarness.resultsstore.storedtestresult.StoredTestResult.check_data",
            return_value=None,
        ) as patch_check_data:
            self.get_stored_test_result(TestDataFilter.ALL, check=True)
        patch_check_data.assert_called_once()

    def test_no_check(self):
        """Test building without checking in the parent result."""
        with patch(
            "TestHarness.resultsstore.storedtestresult.StoredTestResult.check_data",
            return_value=None,
        ) as patch_check_data:
            self.get_stored_test_result(TestDataFilter.ALL, check=False)
        patch_check_data.assert_not_called()

    def test_require_filter(self):
        """Test require_filter()."""
        for test_filter in TestDataFilter:
            if test_filter != TestDataFilter.ALL:
                test, _ = self.get_stored_test_result(test_filter)

                # Correct filter exists
                test._require_filter(test_filter)

                # Raises for all others
                for other_test_filter in TestDataFilter:
                    if (
                        other_test_filter != test_filter
                        and other_test_filter != TestDataFilter.ALL
                    ):
                        with self.assertRaisesRegex(
                            ValueError,
                            f"Test data filter {other_test_filter} "
                            "required and not loaded",
                        ):
                            test._require_filter(other_test_filter)

        # Any filter works when ALL is set
        test, _ = self.get_stored_test_result(TestDataFilter.ALL)
        for test_filter in TestDataFilter:
            if test_filter != TestDataFilter.ALL:
                test._require_filter(test_filter)

    def test_status(self):
        """Test status property."""
        test, _ = self.get_stored_test_result()

        # Exists
        self.assertIn("status", test.data)
        self.assertEqual(test.status, test.data["status"])

        # Doesn't exist
        del test.data["status"]
        self.assertIsNone(test.status)

    def test_status_no_filter(self):
        """Test status property without the STATUS filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "STATUS"):
            test.status

    def test_status_bad_type(self):
        """Test status property with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["status"] = "foo"
        with self.assertRaisesRegex(TypeError, "status"):
            test.status

    def test_status_value(self):
        """Test status_value property."""
        test, _ = self.get_stored_test_result()

        # Exists
        assert test.status is not None
        self.assertIn("status", test.status)
        self.assertEqual(test.status_value, test.status["status"])

        # Doesn't exist
        del test.data["status"]
        self.assertIsNone(test.status_value)

    def test_status_value_no_filter(self):
        """Test status_value property without the STATUS filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "STATUS"):
            test.status_value

    def test_status_value_bad_type(self):
        """Test status_value property with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["status"]["status"] = 1234
        with self.assertRaisesRegex(TypeError, "status"):
            test.status_value

    def test_timing(self):
        """Test timing property."""
        test, _ = self.get_stored_test_result()

        # Exists
        self.assertIn("timing", test.data)
        self.assertEqual(test.timing, test.data["timing"])

        # Doesn't exist
        del test.data["timing"]
        self.assertIsNone(test.timing)

    def test_timing_no_filter(self):
        """Test timing property without the TIMING filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "TIMING"):
            test.timing

    def test_timing_bad_type(self):
        """Test timing property with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["timing"] = "foo"
        with self.assertRaisesRegex(TypeError, "timing"):
            test.timing

    def test_get_timing_entry(self):
        """Test get_timing_entry()."""
        test, _ = self.get_stored_test_result()
        self.assertIn("timing", test.data)
        self.assertGreater(len(test.data["timing"]), 0)
        for key in test.data["timing"]:
            self.assertEqual(test.get_timing_entry(key), test.data["timing"][key])
        self.assertIsNone(test.get_timing_entry("foo"))

    def test_get_timing_entry_no_filter(self):
        """Test get_timing_entry() without the TIMING filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "TIMING"):
            test.get_timing_entry("foo")

    def test_get_timing_entry_bad_type(self):
        """Test get_timing_entry() with bad data."""
        test, _ = self.get_stored_test_result()
        name = "foo"
        test.data["timing"] = {name: "foo"}
        with self.assertRaisesRegex(TypeError, name):
            test.get_timing_entry(name)

    def test_run_time(self):
        """Test run_time property."""
        tests = self.get_stored_test_results()
        for test, _ in tests:
            timing = test.timing
            assert timing is not None
            self.assertEqual(test.run_time, timing.get("runner_run"))

        # Make sure at least one test has run time and one doesn't
        self.assertTrue(any(v[0].run_time is not None for v in tests))
        self.assertTrue(any(v[0].run_time is None for v in tests))

    def test_hpc_queued_time(self):
        """Test hpc_queued_time property."""
        test, _ = self.get_stored_test_result()

        # Doesn't have
        self.assertIsNone(test.hpc_queued_time)

        # Has
        hpc_queued_time = 1.0
        test.data["timing"]["hpc_queued_time"] = hpc_queued_time
        self.assertEqual(test.hpc_queued_time, hpc_queued_time)

    def test_result_properties(self):
        """Test the properties that redirect to the result."""
        test, _ = self.get_stored_test_result()
        for key in [
            "event_sha",
            "event_cause",
            "event_id",
            "pr_num",
            "base_sha",
            "time",
        ]:
            self.assertEqual(getattr(test, key), getattr(test.result, key))

    def test_tester(self):
        """Test tester property."""
        test, _ = self.get_stored_test_result()

        # Exists
        self.assertIn("tester", test.data)
        self.assertEqual(test.tester, test.data["tester"])

        # Doesn't exist
        del test.data["tester"]
        self.assertIsNone(test.tester)

    def test_tester_no_filter(self):
        """Test tester property without the TESTER filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "TESTER"):
            test.tester

    def test_tester_bad_type(self):
        """Test tester property with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["tester"] = "foo"
        with self.assertRaisesRegex(TypeError, "tester"):
            test.tester

    def test_hpc(self):
        """Test hpc property."""
        test, _ = self.get_stored_test_result()

        # Doesn't exist
        self.assertNotIn("hpc", test.data)
        self.assertIsNone(test.hpc)

        # Does exist
        hpc = {"foo": "bar"}
        test.data["hpc"] = hpc
        self.assertEqual(test.hpc, hpc)

    def test_hpc_no_filter(self):
        """Test hpc property without the HPC filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.TESTER)
        with self.assertRaisesRegex(ValueError, "HPC"):
            test.hpc

    def test_hpc_bad_type(self):
        """Test hpc property with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["hpc"] = "foo"
        with self.assertRaisesRegex(TypeError, "hpc"):
            test.hpc

    def test_hpc_id(self):
        """Test hpc_id property."""
        test, _ = self.get_stored_test_result()

        # Doesn't exist
        self.assertNotIn("hpc", test.data)
        self.assertIsNone(test.hpc_id)

        # Does exist
        hpc = {"id": "1234"}
        test.data["hpc"] = hpc
        self.assertEqual(test.hpc_id, hpc["id"])

    def test_hpc_id_no_filter(self):
        """Test hpc_id property without the HPC filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.TESTER)
        with self.assertRaisesRegex(ValueError, "HPC"):
            test.hpc_id

    def test_hpc_id_bad_type(self):
        """Test hpc_id property with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["hpc"] = {"id": 1234}
        with self.assertRaisesRegex(TypeError, "id"):
            test.hpc_id

    def test_validation(self):
        """Test validation property."""
        tests = self.get_stored_test_results()
        for test, _ in tests:
            validation = test.validation
            orig_validation = test.data.get("validation")
            self.assertEqual(validation, orig_validation)
            if orig_validation:
                self.assertIsInstance(validation, dict)
            else:
                self.assertIsNone(validation)

        # Make sure at least one test has validation and one doesn't
        self.assertTrue(any(v[0].validation is not None for v in tests))
        self.assertTrue(any(v[0].validation is None for v in tests))

    def test_validation_no_filter(self):
        """Test validation property without the VALIDATION filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "VALIDATION"):
            test.validation

    def test_validation_bad_type(self):
        """Test validation property with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["validation"] = "foo"
        with self.assertRaisesRegex(TypeError, "validation"):
            test.validation

    def test_get_validation_data(self):
        """Test get_validation_data()."""
        tests = self.get_stored_test_results()
        for test, data in tests:
            validation_data = test.get_validation_data()
            self.assertIsInstance(validation_data, dict)
            orig_validation_data = data.get("validation", {}).get("data", {})
            self.assertAlmostEqual(len(validation_data), len(orig_validation_data))
            for k, v in validation_data.items():
                compare = deepcopy(orig_validation_data[k])
                if (bounds := compare.get("bounds")) is not None:
                    compare["bounds"] = tuple(bounds)
                compare.pop("type")
                compare["validation"] = True
                self.assertEqual(asdict(v), compare)

        # Make sure at least one test has validation data and one doesn't
        self.assertTrue(any(len(v[0].get_validation_data()) > 0 for v in tests))
        self.assertTrue(any(len(v[0].get_validation_data()) == 0 for v in tests))

    def test_get_validation_data_no_type(self):
        """Test get_validation_data() when types were not stored."""
        tests = self.get_stored_test_results(check=False)
        for test, data in tests:
            test.result.data["testharness"]["validation_version"] = 0
            self.assertEqual(test.result.validation_version, 0)

            validation_data = test.data.get("validation", {}).get("data", {})
            for v in validation_data.values():
                del v["type"]

            test.get_validation_data()

        # Make sure at least one test has validation data
        self.assertTrue(any(len(v[0].get_validation_data()) > 0 for v in tests))

    def test_get_validation_data_bad_type(self):
        """Test get_validation_data() with an unknown data type."""
        tests = self.get_stored_test_results()
        for test, _ in tests:
            if validation_data := test.data.get("validation", {}).get("data", {}):
                for value in validation_data.values():
                    bad_type = "foo"
                    value["type"] = bad_type

            if validation_data:
                with self.assertRaisesRegex(
                    ValueError, "Unknown validation data type 'foo'"
                ):
                    test.get_validation_data()

    def test_get_validation_data_no_filter(self):
        """Test get_validation_data() without the VALIDATION filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "VALIDATION"):
            test.get_validation_data()

    def test_get_validation_results(self):
        """Test get_validation_results()."""
        tests = self.get_stored_test_results()
        for test, data in tests:
            validation_results = test.get_validation_results()
            self.assertIsInstance(validation_results, list)
            orig_validation_results = data.get("validation", {}).get("results", {})
            self.assertEqual(len(validation_results), len(orig_validation_results))
            for i, v in enumerate(validation_results):
                compare = deepcopy(orig_validation_results[i])
                compare["validation"] = True
                self.assertEqual(asdict(v), compare)

        # Make sure at least one test has validation results and one doesn't
        self.assertTrue(any(len(v[0].get_validation_results()) > 0 for v in tests))
        self.assertTrue(any(len(v[0].get_validation_results()) == 0 for v in tests))

    def test_get_validation_results_no_filter(self):
        """Test get_validation_results() without the VALIDATION filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "VALIDATION"):
            test.get_validation_results()

    def test_get_json_metadata(self):
        """Test get_json_metadata()."""
        tests = self.get_stored_test_results()
        for test, data in tests:
            orig_tester = data["tester"]
            json_metadata = test.get_json_metadata()
            orig_json_metadata = orig_tester.get("json_metadata", {})
            self.assertEqual(json_metadata, orig_json_metadata)
            self.assertIsInstance(json_metadata, dict)

        # Make sure at least one test has json metadata and one doesn't
        self.assertTrue(any(len(v[0].get_json_metadata()) > 0 for v in tests))
        self.assertTrue(any(len(v[0].get_json_metadata()) == 0 for v in tests))

    def test_get_json_metadata_bad_decompress(self):
        """Test get_json_metadata() when the decompression fails."""
        tests = self.get_stored_test_results(check=False)
        tested = False
        for test, data in tests:
            json_metadata = test.data["tester"].get("json_metadata")
            if json_metadata:
                test.data["tester"]["json_metadata"] = {
                    k: b"1234" for k in json_metadata
                }
                with self.assertRaisesRegex(
                    ValueError, "Failed to decompress json_metadata"
                ):
                    tested = True
                    test.get_json_metadata()

        # Make sure at least one test has json metadata
        self.assertTrue(tested)

    def test_get_json_metadata_no_filter(self):
        """Test get_json_metadata() without the TESTER filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "TESTER"):
            test.get_json_metadata()

    def test_get_json_metadata_bad_type(self):
        """Test get_json_metadata() with bad data."""
        test, _ = self.get_stored_test_result()
        test.data["tester"] = {"json_metadata": 1234}
        with self.assertRaisesRegex(TypeError, "json_metadata"):
            test.get_json_metadata()

    def test_get_perf_graph(self):
        """Test get_perf_graph()."""
        tests = self.get_stored_test_results()
        for test, data in tests:
            orig_tester = data["tester"]
            orig_perf_graph = orig_tester.get("json_metadata", {}).get("perf_graph")
            perf_graph = test.get_perf_graph()
            self.assertEqual(perf_graph, orig_perf_graph)
            if orig_perf_graph:
                self.assertIsInstance(perf_graph, dict)
            else:
                self.assertIsNone(perf_graph)

        # Make sure at least one test has json metadata and one doesn't
        self.assertTrue(any(v[0].get_perf_graph() is not None for v in tests))
        self.assertTrue(any(v[0].get_perf_graph() is None for v in tests))

    def test_get_perf_graph_no_filter(self):
        """Test get_perf_graph() without the TESTER filter."""
        test, _ = self.get_stored_test_result(TestDataFilter.HPC)
        with self.assertRaisesRegex(ValueError, "TESTER"):
            test.get_perf_graph()

    def test_get_max_memory(self):
        """Test get_max_memory()."""
        tests = self.get_stored_test_results()

        max_memory = 1234
        for test, data in tests:
            test.result.data["testharness"]["version"] = 9
            test.data["max_memory"] = max_memory
            self.assertEqual(test.max_memory, max_memory)

            del test.data["max_memory"]
            self.assertIsNone(test.max_memory)

    def test_get_max_memory_bad_version(self):
        """Test get_max_memory()."""
        tests = self.get_stored_test_results()
        for test, data in tests:
            test.result.data["testharness"]["version"] = 2
            self.assertIsNone(test.max_memory)

    def run_test_serialize_deserialize(self, **kwargs):
        """
        Run a test for serialize() and deserialize().

        Keyword arguments are passed to get_stored_test_results().
        """
        for test, _ in self.get_stored_test_results(**kwargs):
            serialized = test.serialize()
            dumped = json.dumps(serialized)
            loaded = json.loads(dumped)
            built = StoredTestResult.deserialize(loaded, test.result)
            self.assertEqual(built.data, test.data)
            self.assertEqual(built.name, test.name)
            self.assertEqual(built.result, test.result)
            self.assertEqual(built._data_filters, test._data_filters)

    def test_serialize_deserialize_tests_within(self):
        """Test serialize() and deserialize() with tests stored within the result."""
        self.run_test_serialize_deserialize(separate_test=False)

    def test_serialize_deserialize_tests_separate(self):
        """Test serialize() and deserialize() with tests stored separately."""
        self.run_test_serialize_deserialize(separate_test=True)

    def test_serialize_deserialize_tests_with_time(self):
        """
        Test serialize() and deserialize() with timing stored in tests.

        This is for deprecated support.
        """
        self.run_test_serialize_deserialize(update_data={"time": datetime.now()})
