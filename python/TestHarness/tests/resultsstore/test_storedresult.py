# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.storedresult.StoredResult."""

from copy import deepcopy
from typing import Optional

from bson.objectid import ObjectId
from TestHarness.resultsstore.civetstore import CIVETStore
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.tests.resultsstore.common import ResultsStoreTestCase
from TestHarness.tests.resultsstore.test_civetstore import (
    base_civet_env,
    build_civet_env,
)

OBJECT_ID = ObjectId()

# A dummy environment from CIVET to build a dummy header
BASE_SHA, CIVET_ENV = build_civet_env()
# A dummy header (entries added to the result entry)
STORE_HEADER = CIVETStore.build_header(BASE_SHA, CIVET_ENV)


def build_stored_result(
    result: dict,
    remove_header: Optional[list[str]] = None,
    header: Optional[dict] = None,
    civet_version: Optional[int] = CIVETStore.CIVET_VERSION,
    remove_testharness: Optional[list[str]] = None,
    update_civet: Optional[dict] = None,
    remove: Optional[list[str]] = None,
    update: Optional[dict] = None,
    check: bool = True,
) -> StoredResult:
    """
    Convert a TestHarness JSON result to a database result.

    Parameters
    ----------
    result : dict
        The results that come from TestHarness JSON result output.

    Optional Parameters
    -------------------
    remove_header : Optional[list[str]]
        Remove these keys from the header.
    header : Optional[dict]
        Use this as the header instead.
    civet_version : int
        Modify the civet_version key.
    remove_testharness : Optional[list[str]]
        Remove these keys from the testharness entry.
    update_civet : Optional[dict]
        Update the civet key with this.
    remove : Optional[list[str]]:
        Remove these keys from the root.
    update : Optional[dict]
        Update the root with this.
    check : bool
        The value of check to pass to the StoredResult.

    """
    result = deepcopy(result)

    # Remove tests; not used here
    del result["tests"]

    # Add an ID
    result["_id"] = OBJECT_ID

    # Add in header, removing keys to be deleted
    header = deepcopy(STORE_HEADER) if not header else deepcopy(header)
    if civet_version:
        header["civet_version"] = civet_version
    else:
        del header["civet_version"]
    if remove_header:
        for k in remove_header:
            del header[k]
    result.update(header)

    # Remove from testharness entry
    if remove_testharness:
        for k in remove_testharness:
            del result["testharness"][k]

    # Update civet
    if update_civet:
        result["civet"].update(update_civet)

    # Remove from root
    if remove:
        for k in remove:
            del result[k]

    # Update root
    if update:
        result.update(update)

    return StoredResult(result, check=check)


class TestStoredResult(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.storedresult.StoredResult."""

    def run_test(self, **kwargs):
        """Run a basic test, building a StoredResult and checking common values."""
        result = self.get_testharness_result("--capture-perf-graph")

        stored_result = build_stored_result(result, **kwargs)
        data = stored_result.data
        testharness = data["testharness"]

        self.assertEqual(stored_result.check, kwargs.get("check", True))
        self.assertEqual(stored_result.data, stored_result._data)
        self.assertEqual(stored_result.id, OBJECT_ID)
        self.assertEqual(stored_result.testharness, testharness)
        self.assertEqual(stored_result.version, testharness["version"])
        self.assertEqual(
            stored_result.validation_version, testharness.get("validation_version", 0)
        )
        self.assertEqual(stored_result.civet, data["civet"])
        self.assertEqual(stored_result.civet_job_id, data["civet"]["job_id"])
        self.assertEqual(stored_result.civet_job_url, data["civet"]["job_url"])
        self.assertEqual(stored_result.event_sha, data["event_sha"])
        self.assertEqual(stored_result.event_cause, data["event_cause"])
        if stored_result.civet_version > 2:
            self.assertIsNotNone(stored_result.event_id)
            self.assertEqual(stored_result.event_id, data["event_id"])
        else:
            self.assertIsNone(stored_result.event_id)
        self.assertEqual(stored_result.pr_num, data["pr_num"])
        if stored_result.civet_version > 1:
            self.assertIsNotNone(stored_result.base_sha)
            self.assertEqual(stored_result.base_sha, data["base_sha"])
        else:
            self.assertIsNone(stored_result.base_sha)
        self.assertEqual(stored_result.time, data["time"])

        return stored_result

    def test_init(self):
        """Test a build of a StoredResult."""
        stored_result = self.run_test()
        self.assertEqual(stored_result.civet_version, CIVETStore.CIVET_VERSION)

    def test_push(self):
        """Test building a StoredResult as a push event."""
        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Push next", "CIVET_PR_NUM": "0"}
        env.update(base_env)

        header = CIVETStore.build_header(base_sha, env)
        stored_result = self.run_test(header=header)
        self.assertIsNone(stored_result.pr_num)
        self.assertEqual(stored_result.event_cause, "push")

    def test_scheduled(self):
        """Test building a StoredResult as a scheduled event."""
        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Scheduled", "CIVET_PR_NUM": "0"}
        env.update(base_env)

        header = CIVETStore.build_header(base_sha, env)
        stored_result = self.run_test(header=header)
        self.assertIsNone(stored_result.pr_num)
        self.assertEqual(stored_result.event_cause, "scheduled")

    def test_hpc(self):
        """Test setting the hpc key and querying it."""
        hpc = {"foo": "bar"}
        stored_result = self.run_test(update={"hpc": hpc})
        self.assertEqual(stored_result.hpc, hpc)

    def test_deprecated_validation_version(self):
        """Test when validation_version didn't exist in the testharness key."""
        stored_result = self.run_test(remove_testharness=["validation_version"])
        self.assertEqual(stored_result.validation_version, 0)

    def test_civet_version_old(self):
        """Test old capabilities for civet_version."""
        # Change civet_version key
        version = CIVETStore.CIVET_VERSION - 1
        stored_result = self.run_test(civet_version=version)
        self.assertEqual(stored_result.civet_version, version)

        # Remove civet_version key and set in ['civet']['version']
        version = 1
        stored_result = self.run_test(
            civet_version=None, update_civet={"version": version}
        )
        self.assertEqual(stored_result.civet_version, version)

        # No version at all
        stored_result = self.run_test(civet_version=None)
        self.assertEqual(stored_result.civet_version, 0)

    def test_event_id_old(self):
        """Test old capabilities for event_id, added in civet version 3."""
        stored_result = self.run_test(civet_version=2, remove_header=["event_id"])
        self.assertIsNone(stored_result.event_id)

    def test_base_sha_old(self):
        """Test old capabilities for base_sha, added in civet version 2."""
        stored_result = self.run_test(civet_version=1, remove_header=["base_sha"])
        self.assertIsNone(stored_result.base_sha)

    def test_no_check(self):
        """Test building with check=False, skipping the check call."""
        # Remove event_sha, which should otherwise fail the check
        result = self.get_testharness_result("--capture-perf-graph")
        stored_result = build_stored_result(result, remove=["event_sha"], check=False)
        with self.assertRaises(KeyError):
            stored_result.check_data()

    def test_check_data(self):
        """Test check data throwing when things are removed."""
        result = self.get_testharness_result("--capture-perf-graph")

        # Remove top level entries
        for key in [
            "_id",
            "testharness",
            "civet",
            "event_sha",
            "event_cause",
            "event_id",
            "base_sha",
            "time",
        ]:
            with self.assertRaisesRegex(KeyError, key):
                build_stored_result(result, remove=[key])
        # Remove testharness entries
        for key in ["version"]:
            with self.assertRaisesRegex(KeyError, key):
                build_stored_result(result, remove_testharness=[key])

        # Bad types
        for key in [
            "_id",
            "testharness",
            "civet",
            "civet_version",
            "hpc",
            "event_sha",
            "event_cause",
            "event_id",
            "pr_num",
            "base_sha",
            "time",
        ]:
            with self.assertRaisesRegex(TypeError, key):
                build_stored_result(result, update={key: 1.0})
