# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test Test TestHarness.resultsstore.utils."""

from TestHarness.resultsstore.utils import (
    MongoPath,
    MutableResultsFolderIterator,
    MutableResultsTestIterator,
    ResultsFolderIterator,
    ResultsTestIterator,
    TestName,
    _type_error_message,
    compress,
    compress_dict,
    decompress,
    decompress_dict,
    get_typed,
    mutable_results_folder_iterator,
    mutable_results_test_iterator,
    results_folder_iterator,
    results_has_test,
    results_num_tests,
    results_query_test,
    results_set_test_value,
    results_test_entry,
    results_test_iterator,
    results_test_names,
)
from TestHarness.tests.resultsstore.common import (
    FOLDER_NAMES,
    NUM_TEST_FOLDERS,
    NUM_TESTS,
    TEST_NAMES,
    ResultsStoreTestCase,
)

# A single test name to test with
TEST_NAME = TEST_NAMES[0]
# A single bad name to test with
BAD_TEST_NAME = TestName("foo", "bar")
assert BAD_TEST_NAME not in TEST_NAMES


class TestUtils(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.utils."""

    def test_MongoPath(self):
        """Test MongoPath()."""
        mp = MongoPath("foo", "bar.baz")
        self.assertEqual(mp.path, ("foo", "bar.baz"))
        self.assertEqual(mp.query_path, "foo.bar\uff0ebaz")
        extended = mp.extend("bang")
        self.assertEqual(extended.path, ("foo", "bar.baz", "bang"))

    def test_TestName(self):
        """Test TestName."""
        name = TestName("foo", "bar")
        self.assertEqual(str(name), "foo.bar")
        self.assertEqual(name.mongo_path.path, ("tests", "foo", "tests", "bar"))

    def test_compress_decompress(self):
        """Test compress() and decompress()."""
        value = "abcd123"
        compressed = compress(value)
        self.assertIsInstance(compressed, bytes)
        decompressed = decompress(compressed)
        self.assertIsInstance(decompressed, str)
        self.assertEqual(value, decompressed)

    def test_compress_decompress_dict(self):
        """Test compress_dict() and decompress_dict()."""
        value = {"foo": "bar"}
        compressed = compress_dict(value)
        self.assertIsInstance(compressed, bytes)
        decompressed = decompress_dict(compressed)
        self.assertIsInstance(decompressed, dict)
        self.assertEqual(value, decompressed)

    def test_results_test_entry(self):
        """Test results_test_entry()."""
        results = self.get_testharness_result()
        value = results["tests"][TEST_NAME.folder]["tests"][TEST_NAME.name]
        self.assertEqual(results_test_entry(results, TEST_NAME), value)

    def test_results_query_test(self):
        """Test results_query_test()."""
        results = self.get_testharness_result()

        # exists
        value = results["tests"][TEST_NAME.folder]["tests"][TEST_NAME.name]
        self.assertEqual(results_query_test(results, TEST_NAME), value)

        # doesn't exist
        self.assertIsNone(results_query_test(results, BAD_TEST_NAME))

    def test_results_has_test(self):
        """Test results_has_test()."""
        results = self.get_testharness_result()
        self.assertTrue(results_has_test(results, TEST_NAME))
        self.assertFalse(results_has_test(results, BAD_TEST_NAME))

    def test_results_set_test_value(self):
        """Test results_set_test_value()."""
        results = self.get_testharness_result()
        results_set_test_value(results, TEST_NAME, "foo")
        self.assertEqual(results_test_entry(results, TEST_NAME), "foo")

    def test_ResultsFolderIterator(self):
        """Test ResultsFolderIterator."""
        tests = [{"foo": "bar"}, {"baz": "bang"}]
        value = {"tests": tests}
        it = ResultsFolderIterator(TEST_NAME.folder, value)
        self.assertEqual(it.name, TEST_NAME.folder)
        self.assertEqual(it.value, value)
        self.assertEqual(it.tests, tests)
        self.assertEqual(it.num_tests, len(tests))

    def test_results_folder_iterator(self):
        """Test results_folder_iterator()."""
        results = self.get_testharness_result()
        its = list(results_folder_iterator(results))
        self.assertEqual(len(its), NUM_TEST_FOLDERS)
        for it in its:
            self.assertIn(it.name, FOLDER_NAMES)
            folder_tests = results["tests"][it.name]["tests"]
            self.assertEqual(it.tests, folder_tests)
            self.assertEqual(it.num_tests, len(folder_tests))

    def test_results_num_tests(self):
        """Test results_num_tests()."""
        results = self.get_testharness_result()
        self.assertEqual(NUM_TESTS, results_num_tests(results))

    def test_ResultsTestIterator(self):
        """Test ResultsTestIterator."""
        value = {"foo": "bar"}
        it = ResultsTestIterator(TEST_NAME, value)
        self.assertEqual(it.name, TEST_NAME)
        self.assertEqual(it.value, value)

    def test_results_test_iterator(self):
        """Tests results_test_iterator."""
        results = self.get_testharness_result()
        its = list(results_test_iterator(results))
        self.assertEqual(len(its), NUM_TESTS)
        for it in its:
            self.assertIsInstance(it, ResultsTestIterator)
            self.assertIn(it.name, TEST_NAMES)

    def test_results_test_names(self):
        """Tests results_test_names()."""
        results = self.get_testharness_result()
        names = results_test_names(results)
        self.assertEqual(NUM_TESTS, len(names))
        for name in names:
            self.assertIn(name, TEST_NAMES)

    def test_MutableResultsTestIterator(self):
        """Test MutableResultsTestIterator."""
        name = "foo"
        values = {name: "bar"}
        it = MutableResultsTestIterator(TEST_NAME.folder, name, values)
        self.assertEqual(it.folder_name, TEST_NAME.folder)
        self.assertEqual(it.test_name, name)
        self.assertEqual(it._folder_tests_entry, values)
        self.assertEqual(it.name, TestName(TEST_NAME.folder, name))
        self.assertEqual(it.value, values[name])

        # Change the value
        it.set_value("baz")
        self.assertEqual(values[name], "baz")

        # Deletet he value
        it.delete()
        self.assertNotIn(name, values)

    def test_mutable_results_test_iterator(self):
        """Test mutable_results_test_iterator()."""
        results = self.get_testharness_result()
        its = list(mutable_results_test_iterator(results))
        self.assertEqual(len(its), NUM_TESTS)
        for it in its:
            self.assertIsInstance(it, MutableResultsTestIterator)
            self.assertIn(it.name, TEST_NAMES)
            self.assertIn(it.name.folder, FOLDER_NAMES)
            folder_tests = results["tests"][it.name.folder]["tests"]
            self.assertEqual(it.value, folder_tests[it.name.name])
            it.set_value("foo")
            self.assertEqual("foo", folder_tests[it.name.name])
            it.delete()
            self.assertNotIn(it.name.name, folder_tests)

    def test_MutableResultsFolderIterator(self):
        """Test MutableResultsFolderIterator."""
        folder_tests = {"foo": "bar", "baz": "bang"}
        results = {"tests": {TEST_NAME.folder: {"tests": folder_tests}}}

        it = MutableResultsFolderIterator(TEST_NAME.folder, results["tests"])
        self.assertEqual(it.name, TEST_NAME.folder)
        self.assertEqual(it._tests_entry, results["tests"])
        self.assertEqual(it.value, results["tests"][TEST_NAME.folder])
        self.assertEqual(it.num_tests, len(folder_tests))
        test_its = list(it.test_iterator())
        self.assertEqual(len(test_its), len(folder_tests))
        it.delete_if_empty()
        for test_it in test_its:
            self.assertEqual(test_it.name.folder, TEST_NAME.folder)
            self.assertEqual(test_it.value, folder_tests[test_it.name.name])
            test_it.delete()
        it.delete_if_empty()
        self.assertNotIn(TEST_NAME.folder, results["tests"])

    def test_mutable_results_folder_iterator(self):
        """Test mutable_results_folder_iterator()."""
        results = self.get_testharness_result()

        its = list(mutable_results_folder_iterator(results))
        self.assertEqual(len(its), NUM_TEST_FOLDERS)
        for it in its:
            self.assertIsInstance(it, MutableResultsFolderIterator)
            self.assertIn(it.name, FOLDER_NAMES)
            it.delete()
            self.assertNotIn(it.name, results["tests"])

    def test_type_error_message(self):
        """Test _type_error_message()."""
        # Single type
        self.assertEqual(
            _type_error_message("key", "value", float),
            "Key 'key' of type str is not of valid type(s): float",
        )
        # Multiple types
        self.assertEqual(
            _type_error_message("key", "value", (float, int)),
            "Key 'key' of type str is not of valid type(s): float, int",
        )

    def test_get_typed(self):
        """Test get_typed()."""
        # Single type okay
        self.assertEqual(get_typed({"foo": "bar"}, "foo", str), "bar")
        # Multiple types okay
        self.assertEqual(get_typed({"foo": "bar"}, "foo", (str, float)), "bar")
        # Doesn't exist, but None is okay
        self.assertIsNone(get_typed({}, "foo", (str, type(None))))
        # Single type bad
        with self.assertRaises(TypeError):
            get_typed({"foo": "bar"}, "foo", int)
        # Multiple types bad
        with self.assertRaises(TypeError):
            get_typed({"foo": "bar"}, "foo", (int, float))
