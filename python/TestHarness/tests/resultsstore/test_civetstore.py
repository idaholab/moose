# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.civetstore.CIVETStore."""

import json
import os
import unittest
from copy import deepcopy
from tempfile import NamedTemporaryFile
from typing import Optional, Tuple

import pytest
from bson.objectid import ObjectId
from mock import patch
from moosepytest.civet import is_civet_pull_request, is_civet_push
from TestHarness.resultsstore.auth import Authentication
from TestHarness.resultsstore.civetstore import (
    MAX_DOCUMENT_SIZE,
    MAX_RESULT_SIZE,
    CIVETStore,
    OversizedResultError,
    OversizedTestsError,
)
from TestHarness.resultsstore.utils import (
    TestName,
    compress_dict,
    results_has_test,
    results_num_tests,
    results_test_entry,
    results_test_iterator,
)
from TestHarness.tests.resultsstore.common import (
    APPLICATION_REPO,
    CIVET_SERVER,
    FakeMongoClient,
    ResultsStoreTestCase,
    base_civet_env,
    build_civet_env,
)

# Dummy database name
TEST_DATABASE = "test_database"

# Name for the live database for storage
LIVE_DATABASE = "civet_tests_moose_resultsstore_civetstore"

# Default arguments to the TestHarness for running runTests()
# that are used by most tests
DEFAULT_TESTHARNESS_ARGS = ["--capture-perf-graph"]

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = CIVETStore.load_authentication() is not None


class TestCIVETStore(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.civetstore.CIVETStore."""

    def test_parse_ssh_repo(self):
        """Test parse_ssh_repo()."""
        self.assertEqual(
            CIVETStore.parse_ssh_repo(APPLICATION_REPO),
            ("github.com", "idaholab", "moose"),
        )

        self.assertEqual(
            CIVETStore.parse_ssh_repo("git@othergithub.host.com:org/repo.git"),
            ("othergithub.host.com", "org", "repo"),
        )

        with self.assertRaises(ValueError):
            CIVETStore.parse_ssh_repo("foo")

    def test_get_civet_repo_url(self):
        """Test get_civet_repo_url()."""
        # Normal PR or push event
        env = {"CIVET_BASE_SSH_URL": APPLICATION_REPO}
        self.assertEqual(
            CIVETStore.get_civet_repo_url(env), "github.com/idaholab/moose"
        )

        # Base will be empty on a scheduled event
        env = {"CIVET_BASE_SSH_URL": "", "APPLICATION_REPO": APPLICATION_REPO}
        self.assertEqual(
            CIVETStore.get_civet_repo_url(env), "github.com/idaholab/moose"
        )

        # Base not set at all
        with self.assertRaisesRegex(KeyError, "CIVET_BASE_SSH_URL not set"):
            CIVETStore.get_civet_repo_url({})

        # Empty base and no APPLICATION_REPO
        env = {"CIVET_BASE_SSH_URL": ""}
        with self.assertRaisesRegex(ValueError, "Failed to obtain repo"):
            CIVETStore.get_civet_repo_url(env)

    def test_get_civet_server(self):
        """Test get_civet_server()."""
        self.assertEqual(CIVETStore.get_civet_server(CIVET_SERVER), "civet.inl.gov")

    @staticmethod
    def build_header_gold(base_sha: str, env: dict) -> dict:
        """Build the base gold for build_header() given a base sha and environment."""
        return {
            "civet": {
                "job_id": int(env["CIVET_JOB_ID"]),
                "job_url": f'civet.inl.gov/job/{env["CIVET_JOB_ID"]}',
                "recipe_name": env["CIVET_RECIPE_NAME"],
                "repo_url": "github.com/idaholab/moose",
                "step": int(env["CIVET_STEP_NUM"]),
                "step_name": env["CIVET_STEP_NAME"],
            },
            "base_sha": base_sha,
            "civet_version": CIVETStore.CIVET_VERSION,
            "event_id": int(env["CIVET_EVENT_ID"]),
            "event_sha": env["CIVET_HEAD_SHA"],
        }

    def test_build_header_pr(self):
        """Test build_header() for a pull request."""
        pr_num = 111
        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Pull request", "CIVET_PR_NUM": str(pr_num)}
        env.update(base_env)

        result = CIVETStore.build_header(base_sha, env)

        gold = self.build_header_gold(base_sha, base_env)
        gold["civet"]["event_url"] = f"github.com/idaholab/moose/pull/{pr_num}"
        gold["civet"]["push_branch"] = env["CIVET_HEAD_REF"]
        gold["event_cause"] = "pr"
        gold["pr_num"] = pr_num
        gold["time"] = result["time"]

        self.assertEqual(result, gold)

    @unittest.skipUnless(
        is_civet_pull_request(),
        "Skipping because not on a CIVET PR",
    )
    def test_build_header_pr_live(self):
        """Test build_header() for a pull request when used on CIVET."""
        env = dict(os.environ)
        pr_num = int(env["CIVET_PR_NUM"])
        base_sha = env["CIVET_BASE_SHA"]

        result = CIVETStore.build_header(base_sha, env)

        gold = self.build_header_gold(base_sha, env)
        gold["civet"]["event_url"] = f"github.com/idaholab/moose/pull/{pr_num}"
        gold["civet"]["push_branch"] = env["CIVET_HEAD_REF"]
        gold["event_cause"] = "pr"
        gold["pr_num"] = pr_num
        gold["time"] = result["time"]

        self.assertEqual(result, gold)

    def test_build_header_push(self):
        """Test build_header() for a push event."""
        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Push next", "CIVET_PR_NUM": "0"}
        env.update(base_env)

        result = CIVETStore.build_header(base_sha, env)

        gold = self.build_header_gold(base_sha, base_env)
        gold["civet"][
            "event_url"
        ] = f'github.com/idaholab/moose/commit/{base_env["CIVET_HEAD_SHA"]}'
        gold["event_cause"] = "push"
        gold["pr_num"] = None
        gold["time"] = result["time"]

        self.assertEqual(result, gold)

    @unittest.skipUnless(
        is_civet_push(),
        "Skipping because not on a CIVET push",
    )
    def test_build_header_push_live(self):
        """Test build_header() for a pull request when used on CIVET."""
        env = dict(os.environ)
        base_sha = env["CIVET_BASE_SHA"]

        result = CIVETStore.build_header(base_sha, env)

        gold = self.build_header_gold(base_sha, env)
        gold["civet"][
            "event_url"
        ] = f'github.com/idaholab/moose/commit/{env["CIVET_HEAD_SHA"]}'
        gold["event_cause"] = "push"
        gold["pr_num"] = None
        gold["time"] = result["time"]

        self.assertEqual(result, gold)

    def test_build_header_scheduled(self):
        """Test build_header() for a scheduled event."""
        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Scheduled", "CIVET_PR_NUM": "0"}
        env.update(base_env)

        result = CIVETStore.build_header(base_sha, env)

        gold = self.build_header_gold(base_sha, base_env)
        gold["civet"][
            "event_url"
        ] = f'github.com/idaholab/moose/commit/{base_env["CIVET_HEAD_SHA"]}'
        gold["event_cause"] = "scheduled"
        gold["pr_num"] = None
        gold["time"] = result["time"]

        self.assertEqual(result, gold)

    def test_build_header_missing_variable(self):
        """Test build_header() when a CIVET variable is missing."""
        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Pull request"}
        env.update(base_env)

        with self.assertRaisesRegex(
            KeyError, "Environment variable CIVET_PR_NUM not set"
        ):
            CIVETStore.build_header(base_sha, env)

    def test_build_header_bad_cause(self):
        """Test build_header() with a bad event cause."""
        base_sha, base_env = base_civet_env()
        env = {"CIVET_EVENT_CAUSE": "Foo", "CIVET_PR_NUM": "0"}
        env.update(base_env)

        with self.assertRaisesRegex(ValueError, 'Unknown event cause "Foo"'):
            CIVETStore.build_header(base_sha, env)

    def _check_result(
        self,
        base_sha: str,
        env: dict,
        result: dict,
        stored_results: dict,
        stored_tests: Optional[dict[TestName, dict]] = None,
        build_kwargs: dict = {},
    ):
        """
        Compare a test harness result to a set of stored results and tests.

        Each test entry is compared directly to the test harness result entry,
        where values are modified/removed as needed based on the arguments
        that are passed to the build method via build_kwargs
        """
        result_id = stored_results.get("_id")
        in_database = result_id is not None

        # Header should exist
        header = CIVETStore.build_header(base_sha, env)
        header["time"] = stored_results["time"]
        for key in header:
            self.assertIn(key, stored_results)
            self.assertEqual(stored_results[key], header[key])

        # Check each test
        for test in results_test_iterator(result):
            test_entry = test.value
            name = test.name
            is_skip = test_entry["status"]["status"] == "SKIP"

            # Ignore skipped tests and this one is skipped
            if build_kwargs.get("ignore_skipped") and is_skip:
                self.assertTrue(not results_has_test(stored_results, name))
                continue

            # Start with the original entry and remove/modify
            # the things that would be different
            modified_test_entry = deepcopy(test_entry)

            stored_test_entry = results_test_entry(stored_results, name)
            if stored_tests:
                if in_database:
                    self.assertIsInstance(stored_test_entry, ObjectId)
                    find_test = [
                        doc for doc in stored_tests if doc["_id"] == stored_test_entry
                    ]
                    self.assertEqual(len(find_test), 1)
                    stored_test = find_test[0]
                    modified_test_entry["_id"] = stored_test_entry
                    modified_test_entry["result_id"] = result_id
                else:
                    self.assertIsNone(stored_test_entry)
                    stored_test = stored_tests[name]
            else:
                stored_test = stored_test_entry
            assert isinstance(stored_test, dict)

            # Output is removed
            for key in ["output", "output_files"]:
                if key in modified_test_entry:
                    del modified_test_entry[key]
                self.assertNotIn(key, stored_test)

            # Only runtime (runner_run timing entry)
            if build_kwargs.get("only_runtime") and not is_skip:
                modified_test_entry["timing"] = {
                    "runner_run": modified_test_entry["timing"]["runner_run"]
                }

            # Removed keys via options
            for key in ["status", "timing", "tester"]:
                if build_kwargs.get(f"ignore_{key}"):
                    del modified_test_entry[key]

            # Compress the JSON metadata
            tester = modified_test_entry.get("tester", {})
            json_metadata = tester.get("json_metadata", {})
            for k, v in json_metadata.items():
                json_metadata[k] = compress_dict(v)

            self.assertEqual(modified_test_entry, stored_test)

    def run_test_build(
        self,
        run_tests_args: Optional[list[str]] = None,
        get_testharness_results_kwargs: Optional[dict] = None,
        separate_tests: bool = False,
        **kwargs,
    ):
        """
        Test the build() method based on a test harness execution.

        The 'build_kwargs' kwarg is passed to the build method and to the check
        method so that things can be checked based on how they are built
        """
        if "build_kwargs" not in kwargs:
            kwargs["build_kwargs"] = {}
        build_kwargs = kwargs["build_kwargs"]

        if separate_tests:
            build_kwargs["max_result_size"] = 1e-6

        base_sha, env = build_civet_env()
        results = self.get_testharness_result(
            *DEFAULT_TESTHARNESS_ARGS,
            *(run_tests_args if run_tests_args else []),
            **(
                get_testharness_results_kwargs if get_testharness_results_kwargs else {}
            ),
        )

        build_result, build_tests = CIVETStore().build(
            results, base_sha=base_sha, env=env, **build_kwargs
        )
        self.assertIsInstance(build_result, dict)
        if separate_tests:
            assert isinstance(build_tests, dict)
            for k, v in build_tests.items():
                self.assertIsInstance(k, TestName)
                self.assertIsInstance(v, dict)
        else:
            self.assertIsNone(build_tests)

        num_tests = results_num_tests(build_result)

        out_regex = rf"Storing {num_tests} tests"
        if build_kwargs.get("ignore_skipped"):
            out_regex += r" \(\d skipped\)"
        out_regex += r" separately" if separate_tests else r" within results"
        out_regex = r"; results size = \d+.\d{2}MB"
        if separate_tests:
            out_regex += r", tests size = \d+.\d{2}MB"

        output = self.stdout_mock.getvalue()
        self.assertRegex(output, out_regex)

        self._check_result(base_sha, env, results, build_result, build_tests, **kwargs)

    def test_build(self):
        """Test build() with no additional arguments and contained tests."""
        self.run_test_build()

    def test_build_sep_files(self):
        """Test build() when the harness is ran with --sep-files."""
        self.run_test_build(run_tests_args=["--sep-files"])

    def test_build_separate_tests(self):
        """Test build() with no additional arguments and separate tests."""
        self.run_test_build(separate_tests=True)

    def test_build_ignore_status(self):
        """Test build() with --ignore-status, not storing the status entry."""
        self.run_test_build(build_kwargs={"ignore_status": True})

    def test_build_ignore_tester(self):
        """Test build() with --ignore-tester, not storing the tester entry."""
        self.run_test_build(build_kwargs={"ignore_tester": True})

    def test_build_ignore_timing(self):
        """Test build() with --ignore-timing, not storing the tester timing."""
        self.run_test_build(build_kwargs={"ignore_timing": True})

    def test_build_only_runtime(self):
        """Test build() with --only-runtime, storing only the runner_time timing."""
        self.run_test_build(build_kwargs={"only_runtime": True})

    def test_build_oversized_test(self):
        """Test build() raising when tests are too big for the database."""
        base_sha, env = build_civet_env()
        results = self.get_testharness_result(*DEFAULT_TESTHARNESS_ARGS)

        bad_test = None
        for test in results_test_iterator(results):
            test.value["foo"] = "a" * int(MAX_DOCUMENT_SIZE * 1024 * 1024 * 1.5)
            bad_test = test.name
            break

        with self.assertRaises(OversizedTestsError) as context:
            CIVETStore().build(results, base_sha=base_sha, env=env)
        tests = context.exception.tests
        self.assertEqual(len(tests), 1)
        self.assertEqual(tests[0][0], bad_test)
        self.assertGreater(tests[0][1], MAX_DOCUMENT_SIZE)

    def test_build_oversized_result(self):
        """Test build() raising when the result is too big for the database."""
        base_sha, env = build_civet_env()
        results = self.get_testharness_result(*DEFAULT_TESTHARNESS_ARGS)

        results["foo"] = "a" * int(MAX_DOCUMENT_SIZE * 1024 * 1024 * 1.5)

        with self.assertRaises(OversizedResultError) as context:
            CIVETStore().build(results, base_sha=base_sha, env=env)
        result_size = context.exception.result_size
        self.assertGreater(result_size, MAX_DOCUMENT_SIZE)

    def test_build_ignore_skipped(self):
        """Test build() with --ignore-skipped, not storing skipped tests."""
        self.run_test_build(
            build_kwargs={"ignore_skipped": True},
        )

    def get_stored_result(
        self, result_id: ObjectId, test_ids: Optional[list[ObjectId]]
    ) -> Tuple[dict, Optional[list[dict]]]:
        """Query the database for a result that has been previously stored."""
        with CIVETStore.setup_client() as client:
            db = client[TEST_DATABASE]

            result_filter = {"_id": {"$eq": result_id}}
            stored_result = db.results.find_one(result_filter)
            assert stored_result is not None

            deleted = db.results.delete_one(result_filter)
            self.assertTrue(deleted.acknowledged)
            self.assertEqual(deleted.deleted_count, 1)

            stored_tests = None
            if test_ids:
                test_filter = {"_id": {"$in": test_ids}}
                stored_tests = [
                    doc for doc in db.tests.find({"_id": {"$in": test_ids}})
                ]
                self.assertEqual(len(stored_tests), len(test_ids))

                deleted = db.tests.delete_many(test_filter)
                self.assertTrue(deleted.acknowledged)
                self.assertTrue(deleted.deleted_count, len(test_ids))

        return stored_result, stored_tests

    def test_setup_client(self):
        """Test setup_client() with a mocked authentication load."""
        auth = Authentication("host", "username", "password", 1234)
        with (
            patch(
                "TestHarness.resultsstore.civetstore.CIVETStore.load_authentication",
                return_value=auth,
            ),
            patch(
                "TestHarness.resultsstore.civetstore.MongoClient",
                return_value=None,
            ) as patched_mongo_client,
        ):
            CIVETStore.setup_client()
            patched_mongo_client.assert_called_once_with(
                auth.host, auth.port, username=auth.username, password=auth.password
            )

    def test_setup_client_not_available(self):
        """Test setup_client() exiting if authentication is not available."""
        with (
            patch(
                "TestHarness.resultsstore.civetstore.CIVETStore.load_authentication",
                return_value=None,
            ),
            self.assertRaisesRegex(
                SystemExit, "ERROR: Authentication is not available"
            ),
        ):
            CIVETStore.setup_client()

    def run_test_build_database(self, separate_tests: bool):
        """Run a test for _build_database(), with or without separate tests."""
        results = self.get_testharness_result()
        num_tests = results_num_tests(results)

        base_sha, env = build_civet_env()
        civetstore = CIVETStore()

        kwargs = {}
        if separate_tests:
            kwargs["max_result_size"] = 1e-6
        built_result, built_tests = civetstore.build(
            results, base_sha, env=env, **kwargs
        )

        db_result, db_tests = civetstore._build_database(built_result, built_tests)
        self.assertIsInstance(db_result, dict)
        self.assertIsInstance(db_tests, list)
        self.assertEqual(len(db_tests), num_tests if separate_tests else 0)
        result_id = db_result["_id"]
        self.assertIsInstance(result_id, ObjectId)

        for test in results_test_iterator(results):
            if separate_tests:
                assert built_tests is not None
                built_test = built_tests[test.name]
                assert isinstance(built_test, dict)

                test_id = results_test_entry(db_result, test.name)
                self.assertIsInstance(test_id, ObjectId)

                db_test = None
                for v in db_tests:
                    if v["_id"] == test_id:
                        db_test = v
                        break
                assert isinstance(db_test, dict)

                self.assertIsInstance(db_test["_id"], ObjectId)
                self.assertEqual(db_test["result_id"], result_id)
                db_test_copy = deepcopy(db_test)
                del db_test_copy["_id"]
                del db_test_copy["result_id"]
                self.assertEqual(db_test_copy, built_test)
            else:
                self.assertIsNone(built_tests)

                built_test = results_test_entry(built_result, test.name)
                db_test = results_test_entry(db_result, test.name)
                self.assertEqual(built_test, db_test)

    def test_build_database(self):
        """Test build_database()."""
        self.run_test_build_database(False)

    def test_build_database_separate_tests(self):
        """Test build_database with separate tests()."""
        self.run_test_build_database(True)

    def run_test_insert_database(self, separate_tests: bool):
        """Run a test for _insert_database(), with or without separate tests."""
        result_id = ObjectId()
        database = "foo"
        result = {"_id": result_id, "foo": "bar"}
        tests = []
        if separate_tests:
            tests = [{"baz": "bang"}]

        civetstore = CIVETStore()
        fake_client = FakeMongoClient()
        with patch.object(civetstore, "setup_client", return_value=fake_client):
            civetstore._insert_database(database, result, tests)

        # Check output
        output = self.stdout_mock.getvalue()
        self.assertIn(f"Inserted result {result_id} into {database}", output)
        if separate_tests:
            self.assertIn(f"Inserted {len(tests)} tests into {database}", output)

        # Check what was stored
        fake_database = fake_client.get_database(database)
        self.assertEqual(fake_database.results.inserted, result)
        if separate_tests:
            self.assertEqual(fake_database.tests.inserted, tests)
        else:
            self.assertIsNone(fake_database.tests.inserted)

    def test_insert_database(self):
        """Test _insert_database()."""
        self.run_test_insert_database(False)

    def test_insert_database_separate_tests(self):
        """Test _insert_database() with separate tests."""
        self.run_test_insert_database(True)

    def run_test_store(self, live: bool, separate_tests: bool = False, **kwargs):
        """Run a test for store(), which builds and inserts into the database."""
        if separate_tests:
            kwargs["max_result_size"] = 1e-6

        base_sha, env = build_civet_env()
        kwargs["env"] = env

        results = self.get_testharness_result()

        civetstore = CIVETStore()

        with patch.object(
            civetstore, "_insert_database", return_value=None
        ) as patch_insert_database:
            result_id, test_ids = civetstore.store(
                TEST_DATABASE, results, base_sha, **kwargs
            )

        self.assertIsInstance(result_id, ObjectId)
        self.assertIsInstance(test_ids, list if separate_tests else type(None))
        if test_ids:
            self.assertEqual(len(test_ids), results_num_tests(results))
            for test_id in test_ids:
                self.assertIsInstance(test_id, ObjectId)

        patch_insert_database.assert_called_once()
        insert_database_args = patch_insert_database.call_args[0]
        self.assertEqual(insert_database_args[0], TEST_DATABASE)
        self.assertIsInstance(insert_database_args[1], dict)
        self.assertIsInstance(insert_database_args[2], list)

    def test_store(self):
        """Test store()."""
        self.run_test_store(False)

    def test_store_separate_tests(self):
        """Test store() with separate tests."""
        self.run_test_store(False, separate_tests=True)

    def test_parse_args(self):
        """Test parse_args()."""
        # Require the result path, base sha, and database
        with self.assertRaises(SystemExit):
            CIVETStore.parse_args([])

        # Success, default args
        base_args = ["result_path", "base_sha", "database"]
        parsed = CIVETStore.parse_args(base_args)
        self.assertEqual(parsed.result_path, "result_path")
        self.assertEqual(parsed.base_sha, "base_sha")
        self.assertEqual(parsed.database, "database")
        self.assertFalse(parsed.ignore_skipped)
        self.assertFalse(parsed.ignore_status)
        self.assertFalse(parsed.ignore_timing)
        self.assertFalse(parsed.ignore_tester)
        self.assertFalse(parsed.only_runtime)
        self.assertEqual(parsed.max_result_size, MAX_RESULT_SIZE)

        # Test setting each boolean
        for entry in [
            "ignore_skipped",
            "ignore_status",
            "ignore_timing",
            "ignore_tester",
            "only_runtime",
        ]:
            args = base_args + [f"--{entry.replace('_', '-')}"]
            parsed = CIVETStore.parse_args(args)
            self.assertTrue(getattr(parsed, entry))

        # max_result_size
        args = base_args + ["--max-result-size=100"]
        parsed = CIVETStore.parse_args(args)
        self.assertEqual(parsed.max_result_size, 100)

    def test_load_authentication(self):
        """Test that load_authentication() calls the underlying helper auth load."""
        with patch(
            "TestHarness.resultsstore.civetstore.auth.load_authentication",
            return_value=None,
        ) as patch_load_authentication:
            CIVETStore.load_authentication()
        patch_load_authentication.assert_called_once_with("CIVET_STORE")

    def test_main_file_missing(self):
        """Test running main() with a results file that doesn't exist."""
        with self.assertRaisesRegex(
            SystemExit, f'Result file {os.path.abspath("foo")} does not exist'
        ):
            CIVETStore().main(
                database="foo", result_path="foo", out_path="", base_sha=""
            )

    def test_main(self):
        """Test main() loading a result and then calling store() on it."""
        base_sha, env = build_civet_env()
        results = self.get_testharness_result()

        with NamedTemporaryFile() as result_file:
            with open(result_file.name, "w") as f:
                json.dump(results, f)

            civetstore = CIVETStore()
            with patch.object(civetstore, "store", return_value=None) as patch_store:
                civetstore.main(result_file.name, TEST_DATABASE, base_sha)

        patch_store.assert_called_once_with(TEST_DATABASE, results, base_sha)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Storer authentication unavailable")
    def test_main_live(self):
        """Test main() loading a result and storing live."""
        base_sha, civet_env = build_civet_env()
        data = self.get_testharness_result()

        civetstore = CIVETStore()

        result_id = None

        try:
            # Run the insert
            with NamedTemporaryFile() as result_file:
                with open(result_file.name, "w") as f:
                    json.dump(data, f)
                result_id, test_ids = civetstore.main(
                    result_file.name, LIVE_DATABASE, base_sha, env=civet_env
                )

            # Should have one inserted ID and no tests (tests contained within)
            self.assertIsInstance(result_id, ObjectId)
            self.assertIsNone(test_ids)

            # Make sure the data exists in the database
            with civetstore.setup_client() as client:
                result = client[LIVE_DATABASE].results.find_one({"_id": result_id})
            self.assertIsNotNone(result)

            # Result should have contained tests
            self.assertIsNotNone(result)
            for test in results_test_iterator(data):
                result_data = results_test_entry(result, test.name)
                self.assertIsInstance(result_data, dict)

        # Delete the entry
        finally:
            if result_id is not None:
                with civetstore.setup_client() as client:
                    client[LIVE_DATABASE].results.delete_one({"_id": result_id})

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Storer authentication unavailable")
    def test_main_live_separate_tests(self):
        """Test main() loading a result and storing live with separate tests."""
        base_sha, civet_env = build_civet_env()
        data = self.get_testharness_result()

        civetstore = CIVETStore()

        result_id = None
        test_ids = None

        try:
            # Run the insert
            with NamedTemporaryFile() as result_file:
                with open(result_file.name, "w") as f:
                    json.dump(data, f)
                result_id, test_ids = civetstore.main(
                    result_file.name,
                    LIVE_DATABASE,
                    base_sha,
                    env=civet_env,
                    max_result_size=1e-6,
                )

            # Should have one inserted ID and no tests (tests contained within)
            self.assertIsInstance(result_id, ObjectId)
            self.assertIsInstance(test_ids, list)
            self.assertTrue(all(isinstance(v, ObjectId) for v in test_ids))

            # Get result and tests
            with civetstore.setup_client() as client:
                db = client[LIVE_DATABASE]
                result = db.results.find_one({"_id": result_id})
                tests = [v for v in db.tests.find({"_id": {"$in": test_ids}})]

            # Result should have separate tests
            self.assertIsNotNone(result)
            for test in results_test_iterator(data):
                result_data = results_test_entry(result, test.name)
                self.assertIsInstance(result_data, ObjectId)

            # Test entries should link to the result
            self.assertEqual(len(tests), len(test_ids))
            for test in tests:
                self.assertEqual(test["result_id"], result_id)
                self.assertIn(test["_id"], test_ids)

        # Delete the entries
        finally:
            if result_id is not None:
                with civetstore.setup_client() as client:
                    client[LIVE_DATABASE].results.delete_one({"_id": result_id})
            if test_ids is not None:
                with civetstore.setup_client() as client:
                    client[LIVE_DATABASE].tests.delete_many({"_id": {"$in": test_ids}})
