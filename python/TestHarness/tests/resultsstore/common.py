# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Contains common testing utilities for TestHarness.resultsstore."""

import os
import random
import string
from dataclasses import dataclass
from io import StringIO
from typing import Any, Iterator, Optional, Tuple
from unittest import TestCase

from bson.objectid import ObjectId
from mock import patch
from pymongo import MongoClient
from pymongo.database import Database
from TestHarness.resultsstore.civetstore import CIVETStore
from TestHarness.resultsstore.utils import TestName
from TestHarness.tests.common import TestHarnessResultCache

# Dummy APPLICATION_REPO variable from the civet environment
APPLICATION_REPO = "git@github.com:idaholab/moose"
# Dummy CIVET_SERVER variable used in the civet environment
CIVET_SERVER = "https://civet-be.inl.gov"

# Static object id for use in testing
OBJECT_ID = ObjectId()


def random_git_sha() -> str:
    """Generate a random Git SHA."""
    hex_characters = string.hexdigits[:16]
    random_sha = "".join(random.choice(hex_characters) for _ in range(40))
    return random_sha


def random_id() -> int:
    """Generate a random ID."""
    return random.randint(1, 1000000)


def base_civet_env() -> Tuple[str, dict]:
    """Build a random base CIVET environment for testing."""
    # Dummy sha to use for the base commit
    base_sha = random_git_sha()
    # Dummy civet environment
    env = {
        "APPLICATION_REPO": APPLICATION_REPO,
        "CIVET_BASE_SHA": random_git_sha(),
        "CIVET_BASE_SSH_URL": "git@github.com:idaholab/moose.git",
        "CIVET_EVENT_ID": str(random_id()),
        "CIVET_HEAD_REF": "branchname",
        "CIVET_HEAD_SHA": random_git_sha(),
        "CIVET_JOB_ID": str(random_id()),
        "CIVET_RECIPE_NAME": "Awesome recipe",
        "CIVET_SERVER": CIVET_SERVER,
        "CIVET_STEP_NAME": "Cool step",
        "CIVET_STEP_NUM": str(random_id()),
    }
    return base_sha, env


def build_civet_env(pr: bool = True) -> Tuple[str, dict]:
    """Build a random base CIVET envrionment used for testing build()."""
    base_sha, civet_env = base_civet_env()
    if pr:
        civet_env.update(
            {"CIVET_EVENT_CAUSE": "Pull request", "CIVET_PR_NUM": str(random_id())}
        )
    else:
        civet_env.update({"CIVET_EVENT_CAUSE": "Push", "CIVET_PR_NUM": ""})
    return base_sha, civet_env


# Cache for the TestHarness results
TESTHARNESS_RESULTS: dict = {}
# Path to the stored results file for when moose is not available
TESTHARNESS_RESULTS_FILE = os.path.join(
    os.path.dirname(__file__), "content", "testharness_results.json"
)
# Whether or not to overwrite the above file
TESTHARNESS_RESULTS_WRITE = False

# Expected test names in the results for testing
TEST_NAMES = [
    TestName("basic", "ok"),
    TestName("basic", "fail"),
    TestName("basic", "skip"),
    TestName("validation", "test"),
]
# Expected folder names in the results for testing
FOLDER_NAMES = list(set(v.folder for v in TEST_NAMES))
# Expected number of tests in the results for testing
NUM_TESTS = len(TEST_NAMES)
# Expected number of folders in the results for testing
NUM_TEST_FOLDERS = len(FOLDER_NAMES)

# Common directory where test content should go
CONTENT_DIR = os.path.join(os.path.dirname(__file__), "content")

# Set to True to rewrite the cache
CACHED_TESTHARNESS_RESULTS_WRITE = False


class ResultsStoreTestCase(TestCase, TestHarnessResultCache):
    """Common test case for testing TestHarness.resultsstore."""

    def __init__(self, *args, **kwargs):
        """Initialize state."""
        # TestCase initializer
        TestCase.__init__(self, *args, **kwargs)

        # Setup cache for test harness results
        TestHarnessResultCache.__init__(
            self,
            os.path.join(CONTENT_DIR, "cached_testharness_results.json"),
            os.path.join(CONTENT_DIR, "testharness_results"),
            "resultsstore",
            CACHED_TESTHARNESS_RESULTS_WRITE,
        )

    def setUp(self):
        """Add a patcher for mocking stdout."""
        # Mock stdout so we can more easily get print statements
        self._stdout_patcher = patch("sys.stdout", new=StringIO())
        self.stdout_mock: StringIO = self._stdout_patcher.start()

    def tearDown(self):
        """Stop the stdout patcher."""
        self._stdout_patcher.stop()

    def get_result_data(self, pr: bool = True) -> dict:
        """Get dummy data for a result as it would be stored."""
        data = self.get_testharness_result()

        # Remove tests entry
        del data["tests"]
        # Add ID
        data["_id"] = OBJECT_ID
        # Add CIVET header
        data.update(CIVETStore.build_header(*build_civet_env(pr=pr)))

        return data


@dataclass
class FakeMongoResult:
    """Fake mongodb insert result for testing."""

    inserted_id: Optional[int] = None
    acknowledged: bool = True


class FakeMongoCursorIterator:
    """Fake iterator for going through a mongo cursor."""

    def __init__(self, values: list[Any]):
        """Initialize state; set values."""
        self.values = values

    def __iter__(self) -> Iterator[Any]:
        """Iterate through the values."""
        yield from self.values


class FakeMongoFind:
    """Fake context for find in a mongo collection."""

    def __init__(self, values: list[Any]):
        """Initialize state; set values."""
        self.values = values

    def __enter__(self) -> FakeMongoCursorIterator:
        """Fake enter; return the iterator."""
        return FakeMongoCursorIterator(self.values)

    def __exit__(self, *_, **__):
        """Fake exit; do nothing."""
        pass


@dataclass
class FakeMongoCollection:
    """Fake mongodb collection for testing."""

    inserted = None
    find_values = None

    def insert_one(self, value):
        """Mimic insert_one."""
        self.inserted = value
        return FakeMongoResult(value["_id"])

    def insert_many(self, value):
        """Mimic insert_many."""
        self.inserted = value
        return FakeMongoResult()

    def find(self, *_, **__):
        """Mimic find."""
        pass

    def aggregate(self, *_, **__):
        """Mimic aggregate."""
        pass


@dataclass
class FakeMongoDatabase(Database):
    """Fake mongodb Database for testing."""

    def __init__(self, *_, **__):
        """Init; do nothing."""
        pass

    # Results collection
    results = FakeMongoCollection()
    # Tests collection
    tests = FakeMongoCollection()


class FakeMongoClient(MongoClient):
    """Fake MongoClient for testing."""

    def __init__(self, *_, **__):
        """Initialize state."""
        # Database name -> fake database
        self._databases: dict[str, FakeMongoDatabase] = {}

    def get_database(self, name, *_, **__) -> FakeMongoDatabase:
        """Get a database."""
        if name not in self._databases:
            self._databases[name] = FakeMongoDatabase()
        return self._databases[name]

    def __getitem__(self, name: str) -> FakeMongoDatabase:
        """Get a database."""
        return self.get_database(name)

    def __enter__(self) -> "FakeMongoClient":
        """Mimic context manager enter; just returns the client."""
        return self

    def __exit__(self, *_):
        """Mimic conext manager exit; does nothing."""
        pass

    def close(self):
        """Close; don't do anything."""
        pass


# Test database name for testing pull request results
TEST_DATABASE_NAME = "civet_tests_moose_store_results_live"
# The name of the test to use in the test database
TEST_DATABASE_TEST_NAME = TestName("tests/test_harness", "ok")

# Production database name for testing golds
GOLD_DATABASE_NAME = "civet_tests_moose_performance"
# The name of the test to use in the gold database
GOLD_DATABASE_TEST_NAME = TestName("simple_transient_diffusion", "test")


@dataclass
class GoldResult:
    """Storage for a gold test."""

    # The result ID
    id: ObjectId
    # The expected civet_version
    civet_version: int
    # The event sha for the result
    event_sha: str
    # The event ID for the result (if any)
    event_id: Optional[int] = None
    # The ID for the test, if stored separately
    test_id: Optional[ObjectId] = None


# Static set of results from which to build the gold file
GOLD_RESULTS = [
    GoldResult(
        id=ObjectId("6857a572bbcb03d9dccfb1a8"),
        civet_version=0,
        event_sha="968f537a3c89ffb556fb7da18da28da52b592ee0",
        test_id=ObjectId("6857a572bbcb03d9dccfb1a7"),
    ),
    GoldResult(
        id=ObjectId("685c623b4022db39df9590c4"),
        civet_version=0,
        event_sha="3d48fa82c081e141fd86390dfb9edd1f11c984ca",
        test_id=ObjectId("685c623b4022db39df9590c3"),
    ),
    GoldResult(
        id=ObjectId("685b0fdf4110325560e2cc30"),
        civet_version=1,
        event_sha="45b8a536530388e7bb1ff563398b1e94f2d691fc",
        test_id=ObjectId("685b0fdf4110325560e2cc2f"),
    ),
    # bump to civet_version=2
    GoldResult(
        id=ObjectId("68658fcbc8ec62f893b8e307"),
        civet_version=2,
        event_sha="1134c7e383972783be2ea702f2738ece79fe6a59",
        test_id=ObjectId("68658fcac8ec62f893b8e306"),
    ),
    # bump to civet_version=3
    # - added event_id
    GoldResult(
        id=ObjectId("68dac86a57e68e67a2888a74"),
        civet_version=3,
        event_id=258481,
        event_sha="d64fb221531abf740a7917376216f4e03517fc80",
        test_id=ObjectId("68dac86a57e68e67a2888a73"),
    ),
    # bump to civet_version=4
    # - remove indices from tests
    GoldResult(
        id=ObjectId("68dedf7cbe5697c197720bae"),
        civet_version=4,
        event_id=259309,
        event_sha="296c9e817d13b35624be2423775b309a34d9336c",
        test_id=ObjectId("68dedf7cbe5697c197720bad"),
    ),
    # bump to civet_version=5
    GoldResult(
        id=ObjectId("68e6141590b44073432086b9"),
        civet_version=5,
        event_id=260038,
        event_sha="810a5570266435660fedf3c207e5b5a640699187",
        test_id=ObjectId("68e6141590b44073432086b8"),
    ),
    # bump to civet_version=6
    # - tests can be stored with results
    # - store moved to CIVETStore object
    GoldResult(
        id=ObjectId("68e7248e56c7e7f826a95d15"),
        civet_version=6,
        event_id=260206,
        event_sha="61f2cba5825555e88e18564d03c54ca1959a8957",
    ),
]
