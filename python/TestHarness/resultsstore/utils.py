#!/usr/bin/env python3
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
from typing import Any, Iterator


@dataclass(frozen=True)
class TestName:
    """Name for a test (folder and name)."""

    folder: str
    name: str

    def __str__(self):
        """Build the combined test name."""
        return f"{self.folder}.{self.name}"


def decompress_dict(compressed: bytes) -> dict:
    """Decompress a dict that was compressed with compress_dict()."""
    assert isinstance(compressed, bytes)
    return json.loads(zlib.decompress(compressed).decode("utf-8"))


def compress_dict(value: dict) -> bytes:
    """Compress a dict into binary."""
    assert isinstance(value, dict)
    return zlib.compress(json.dumps(value).encode("utf-8"))


def results_folder_entry(results: dict, folder_name: str) -> dict:
    """Get the given folder's entry in TestHarness JSON results."""
    assert isinstance(results, dict)
    assert isinstance(folder_name, str)
    return results["tests"][folder_name]


def results_test_entry(results: dict, name: TestName) -> Any:
    """Get the given test's entry in TestHarness JSON results."""
    assert isinstance(results, dict)
    assert isinstance(name, TestName)
    return results["tests"][name.folder]["tests"][name.name]


def results_has_folder(results: dict, folder: str) -> bool:
    """Check whether or not the TestHarness JSON results has the given test folder."""
    assert isinstance(results, dict)
    assert isinstance(folder, str)
    return folder in results["tests"]


def results_has_test(results: dict, name: TestName) -> bool:
    """Check whether or not the TestHarness JSON results has the given test."""
    return (
        results_has_folder(results, name.folder)
        and name.name in results_folder_entry(results, name.folder)["tests"]
    )


def results_set_test_value(results: dict, name: TestName, value: Any):
    """Set the test value for a given test in TestHarness JSON results."""
    results["tests"][name.folder]["tests"][name.name] = value


@dataclass
class ResultsTestIterator:
    """
    Iterator that represents a single test entry in TestHarness results JSON output.

    Enables modification while iterating through the tests entry in the results.
    """

    # The name of the folder the test is in
    folder_name: str
    # The name of the test
    test_name: str
    # Internal storage of the parent folder tests entry for manipulation
    _folder_tests_entry: dict
    # Whether or not delete() was called
    _deleted: bool = False

    @property
    def name(self) -> TestName:
        """Get the combined name (folder name + test name) of the test."""
        return TestName(self.folder_name, self.test_name)

    @property
    def value(self) -> Any:
        """Get the value of the test entry in the dict."""
        assert not self._deleted
        return self._folder_tests_entry[self.test_name]

    def set_value(self, value: Any):
        """Set the value of the test entry in the dict."""
        assert not self._deleted
        self._folder_tests_entry[self.test_name] = value

    def delete(self):
        """Delete the test entry in the dict."""
        assert not self._deleted
        del self._folder_tests_entry[self.test_name]
        self._deleted = True


@dataclass
class ResultsFolderIterator:
    """
    Iterator that represents a single test folder in TestHarness results JSON output.

    Enables modification while iterating through the tests entry in the results.
    """

    # The name of the folder
    name: str
    # Internal storage of the parent tests entry for manipulation
    _tests_entry: dict
    # Whether or not delete() was called
    _deleted: bool = False

    @property
    def value(self):
        """Get the value of the test entry in the dict."""
        assert not self._deleted
        return self._tests_entry[self.name]

    @property
    def tests(self):
        """Get the underlying tests in the dict."""
        return self.value["tests"]

    def test_iterator(self) -> Iterator[ResultsTestIterator]:
        """Create an iterator over the underlying tests."""
        for test_name in list(self.tests.keys()):
            yield ResultsTestIterator(self.name, test_name, self.tests)

    def delete(self):
        """Delete the folder entry in the tests dict."""
        assert not self._deleted
        del self._tests_entry[self.name]
        self._deleted = True

    def delete_if_empty(self):
        """Delete the folder entry in the tests dict if it has no tests."""
        assert not self._deleted
        if not self.tests:
            self.delete()


def results_folder_iterator(results: dict) -> Iterator[ResultsFolderIterator]:
    """
    Iterate through test folder entries in TestHarness results JSON output.

    Enables modification while iterating.
    """
    tests_entry = results["tests"]
    for folder_name in list(tests_entry.keys()):
        yield ResultsFolderIterator(folder_name, tests_entry)


def results_test_iterator(results: dict) -> Iterator[ResultsTestIterator]:
    """
    Iterate through test entries in TestHarness results JSON output.

    Enables modification while iterating.
    """
    for folder in results_folder_iterator(results):
        yield from folder.test_iterator()
