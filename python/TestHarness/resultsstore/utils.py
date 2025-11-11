# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements utilities for working with test results stored in a database."""

import json
import zlib
from dataclasses import dataclass
from typing import Any, Iterator, Optional, Tuple, Type


@dataclass
class MongoPath:
    """Represents a path in a mongo database document."""

    def __init__(self, *args: str):
        """
        Initialize state.

        Arguments:
        ---------
        *args : str
            The elements in the path.

        """
        assert args
        self._path: Tuple[str, ...] = args

    @property
    def path(self) -> Tuple[str, ...]:
        """Get the path."""
        return self._path

    @property
    def query_path(self) -> str:
        """
        Get the mongodb string path for querying this path.

        Replaces "." (which is a separator for keys) with a
        literal dot character.
        """
        clean = [v.replace(".", "\uff0e") for v in self.path]
        return ".".join(clean)

    def extend(self, *args: str) -> "MongoPath":
        """
        Create a new MongoPath with this path extended.

        Arguments:
        ---------
        *args : str
            The elements in the path.

        """
        return MongoPath(*self.path, *args)


@dataclass(frozen=True)
class TestName:
    """Name for a test (folder and name)."""

    __test__ = False  # prevents pytest collection

    # Name of the folder
    folder: str
    # Name of the test in the folder
    name: str

    def __str__(self):
        """Build the combined test name."""
        return f"{self.folder}.{self.name}"

    @property
    def mongo_path(self) -> MongoPath:
        """Get the path in the database for this test."""
        return MongoPath("tests", self.folder, "tests", self.name)


def compress(value: str) -> bytes:
    """Compress a string into binary."""
    assert isinstance(value, str)
    return zlib.compress(value.encode("utf-8"))


def compress_dict(value: dict) -> bytes:
    """Compress a dict into binary."""
    assert isinstance(value, dict)
    return compress(json.dumps(value))


def decompress(compressed: bytes) -> str:
    """Decompress a string that was compressed with compress()."""
    assert isinstance(compressed, bytes)
    return zlib.decompress(compressed).decode("utf-8")


def decompress_dict(compressed: bytes) -> dict:
    """Decompress a dict that was compressed with compress_dict()."""
    assert isinstance(compressed, bytes)
    return json.loads(decompress(compressed))


def results_test_entry(results: dict, name: TestName) -> Any:
    """Get the given test's entry in TestHarness JSON results."""
    assert isinstance(results, dict)
    assert isinstance(name, TestName)
    return results["tests"][name.folder]["tests"][name.name]


def results_query_test(results: dict, name: TestName) -> Optional[dict]:
    """Query a test in the TestHarness JSON results."""
    assert isinstance(results, dict)
    assert isinstance(name, TestName)

    if (folder := results["tests"].get(name.folder)) and (
        test := folder["tests"].get(name.name)
    ):
        return test
    return None


def results_has_test(results: dict, name: TestName) -> bool:
    """Check whether or not the TestHarness JSON results has the given test."""
    return results_query_test(results, name) is not None


def results_set_test_value(results: dict, name: TestName, value: Any):
    """Set the test value for a given test in TestHarness JSON results."""
    results["tests"][name.folder]["tests"][name.name] = value


@dataclass
class ResultsFolderIterator:
    """Iterator for a test folder in TestHarness results JSON output."""

    # The name of the folder
    name: str
    # The value for the folder
    value: dict

    @property
    def tests(self):
        """Get the underlying test data for the test folder."""
        return self.value["tests"]

    @property
    def num_tests(self):
        """Get the number of tests in the test folder."""
        return len(self.tests)


def results_folder_iterator(results: dict) -> Iterator[ResultsFolderIterator]:
    """Iterate through test folder entries in TestHarness results JSON output."""
    for name, value in results["tests"].items():
        yield ResultsFolderIterator(name, value)


def results_num_tests(results: dict) -> int:
    """Get the total number of tests in TestHarness results JSON output."""
    return sum(it.num_tests for it in results_folder_iterator(results))


@dataclass
class ResultsTestIterator:
    """Iterator for a test entry in TestHarness results JSON output."""

    # The combined test name
    name: TestName
    # The test value
    value: dict


def results_test_iterator(
    results: dict,
) -> Iterator[ResultsTestIterator]:
    """Iterate through test entries in TestHarness results JSON output."""
    for folder_name, folder_entry in results["tests"].items():
        for test_name, test_value in folder_entry["tests"].items():
            yield ResultsTestIterator(TestName(folder_name, test_name), test_value)


def results_test_names(results: dict) -> list[TestName]:
    """Get the names of all tests in TestHarness results JSON output."""
    return [v.name for v in results_test_iterator(results)]


@dataclass
class MutableResultsTestIterator:
    """
    Iterator for a mutable test entry in TestHarness results JSON output.

    Enables modification while iterating through the tests entry in the results.
    """

    # The name of the folder the test is in
    folder_name: str
    # The name of the test
    test_name: str
    # Internal storage of the parent folder tests entry for manipulation
    _folder_tests_entry: dict

    @property
    def name(self) -> TestName:
        """Get the combined name (folder name + test name) of the test."""
        return TestName(self.folder_name, self.test_name)

    @property
    def value(self) -> Any:
        """Get the value of the test entry in the dict."""
        return self._folder_tests_entry[self.test_name]

    def set_value(self, value: Any):
        """Set the value of the test entry in the dict."""
        self._folder_tests_entry[self.test_name] = value

    def delete(self):
        """Delete the test entry in the dict."""
        del self._folder_tests_entry[self.test_name]


def mutable_results_test_iterator(
    results: dict,
) -> Iterator[MutableResultsTestIterator]:
    """
    Iterate through mutable test entries in TestHarness results JSON output.

    Enables modification while iterating.
    """
    tests_entry = results["tests"]
    for folder_name, folder_entry in tests_entry.items():
        folder_tests_entry = folder_entry["tests"]
        for test_name in list(folder_tests_entry.keys()):
            yield MutableResultsTestIterator(folder_name, test_name, folder_tests_entry)


@dataclass
class MutableResultsFolderIterator:
    """
    Iterator for a mutable test folder in TestHarness results JSON output.

    Enables modification while iterating through the tests entry in the results.
    """

    # The name of the folder
    name: str
    # Internal storage of the parent tests entry for manipulation
    _tests_entry: dict

    @property
    def value(self):
        """Get the value of the test entry in the dict."""
        return self._tests_entry[self.name]

    @property
    def tests(self):
        """Get the underlying tests in the dict."""
        return self.value["tests"]

    @property
    def num_tests(self):
        """Get the number of tests in the dict."""
        return len(self.tests)

    def test_iterator(self) -> Iterator[MutableResultsTestIterator]:
        """Create an iterator over the underlying tests."""
        for test_name in list(self.tests.keys()):
            yield MutableResultsTestIterator(self.name, test_name, self.tests)

    def delete(self):
        """Delete the folder entry in the tests dict."""
        del self._tests_entry[self.name]

    def delete_if_empty(self):
        """Delete the folder entry in the tests dict if it has no tests."""
        if not self.tests:
            self.delete()


def mutable_results_folder_iterator(
    results: dict,
) -> Iterator[MutableResultsFolderIterator]:
    """
    Iterate through mutable test folder entries in TestHarness results JSON output.

    Enables modification while iterating.
    """
    tests_entry = results["tests"]
    for folder_name in list(tests_entry.keys()):
        yield MutableResultsFolderIterator(folder_name, tests_entry)


def _type_error_message(key: Any, value: Any, types: Type | Tuple[Type, ...]) -> str:
    """Produce an error message for get_typed()."""
    valid_types = ", ".join(
        [v.__name__ for v in (list(types) if isinstance(types, Tuple) else [types])]
    )
    return (
        f"Key '{key}' of type {type(value).__name__} is "
        f"not of valid type(s): {valid_types}"
    )


def get_typed(
    container: dict,
    key: Any,
    types: Type | Tuple[Type, ...],
    default: Any = None,
    allow_missing: bool = False,
) -> Any:
    """
    Get a value from a container given a key, requiring a type.

    Arguments:
    ---------
    container : dict
        The container to search in.
    key : Any
        The key to search in the container.
    types : Type | Tuple[Type, ...]
        The valid types.

    Optional arguments:
    ------------------
    default : Any
        The default value; defaults to None.
    allow_missing : bool
        Whether or not to allow a missing key.

    """
    value = container.get(key, default)
    if not isinstance(value, types):
        if key not in container:
            if allow_missing:
                return value
            else:
                raise KeyError(f"Missing key '{key}'")
        raise TypeError(_type_error_message(key, value, types))
    return value
