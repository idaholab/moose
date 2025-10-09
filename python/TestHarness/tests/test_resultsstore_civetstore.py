#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from tempfile import NamedTemporaryFile
import json
import os
import string
import random
from mock import patch
from io import StringIO
from copy import deepcopy
from typing import Optional, Tuple
from bson.objectid import ObjectId

from TestHarness.resultsstore.civetstore import CIVETStore, MAX_DOCUMENT_SIZE, \
    OversizedTestsError, OversizedResultError
from TestHarness.resultsstore.utils import TestName, decompress_dict, compress_dict, \
    results_has_test, results_test_entry, results_test_iterator
from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = CIVETStore.has_authentication()

# Name for the database used for testing database store
TEST_DATABASE = 'civet_tests_moose_resultsstore_civetstore'

# Generate a random Git SHA
def random_git_sha() -> str:
    hex_characters = string.hexdigits[:16]
    random_sha = ''.join(random.choice(hex_characters) for _ in range(40))
    return random_sha
# Generate a random ID
def random_id() -> int:
    return random.randint(1, 1000000)

# Dummy APPLICATION_REPO variable from the civet environment
APPLICATION_REPO = 'git@github.com:idaholab/moose'
# Dummy CIVET_SERVER variable used in the civet environment
CIVET_SERVER = 'https://civet-be.inl.gov'

# Build a random base CIVET environment
def base_civet_env() -> Tuple[str, dict]:
    # Dummy sha to use for the base commit
    base_sha = random_git_sha()
    # Dummy civet environment
    env = {
        'APPLICATION_REPO': APPLICATION_REPO,
        'CIVET_BASE_SHA': random_git_sha(),
        'CIVET_BASE_SSH_URL': 'git@github.com:idaholab/moose.git',
        'CIVET_EVENT_ID': str(random_id()),
        'CIVET_HEAD_REF': 'branchname',
        'CIVET_HEAD_SHA': random_git_sha(),
        'CIVET_JOB_ID': str(random_id()),
        'CIVET_RECIPE_NAME': 'Awesome recipe',
        'CIVET_SERVER': CIVET_SERVER,
        'CIVET_STEP_NAME': 'Cool step',
        'CIVET_STEP_NUM': str(random_id())
    }
    return base_sha, env

# Build a random base CIVET envrionment used for testing build()
def build_civet_env() -> Tuple[str, dict]:
    env = {
        'CIVET_EVENT_CAUSE': 'Pull request',
        'CIVET_PR_NUM': str(random_id())
    }
    base_sha, base_env = base_civet_env()
    env.update(base_env)
    return base_sha, env

# Default arguments to the TestHarness for running runTests()
# that are used by most tests
DEFAULT_TESTHARNESS_ARGS = ['-i', 'validation', '--capture-perf-graph']
DEFAULT_TESTHARNESS_KWARGS = {'exit_code': 132}

class TestCIVETStore(TestHarnessTestCase):
    def setUp(self):
        # Mock stdout so we can more easily get print statements
        self._stdout_patcher = patch("sys.stdout", new=StringIO())
        self.stdout_mock: StringIO = self._stdout_patcher.start()

    def tearDown(self):
        self._stdout_patcher.stop()

    def testCompressDict(self):
        """
        Tests compress_dic() and decompress_dict()
        """
        value = {'foo': 'bar'}
        compressed_value = compress_dict(value)
        self.assertIsInstance(compressed_value, bytes)
        decompressed_value = decompress_dict(compressed_value)
        self.assertEqual(value, decompressed_value)

    def testParseSSHRepo(self):
        """
        Tests parse_ssh_repo()
        """
        self.assertEqual(
            CIVETStore.parse_ssh_repo(APPLICATION_REPO),
            ('github.com', 'idaholab', 'moose')
        )

        self.assertEqual(
            CIVETStore.parse_ssh_repo('git@othergithub.host.com:org/repo.git'),
            ('othergithub.host.com', 'org', 'repo')
        )

        with self.assertRaises(ValueError):
            CIVETStore.parse_ssh_repo('foo')

    def testGetGetCIVETRepoUrl(self):
        """
        Tests get_civet_repo_url()
        """
        env = {'CIVET_BASE_SSH_URL': APPLICATION_REPO}
        self.assertEqual(
            CIVETStore.get_civet_repo_url(env),
            'github.com/idaholab/moose'
        )

        env = {'APPLICATION_REPO': APPLICATION_REPO}
        self.assertEqual(
            CIVETStore.get_civet_repo_url(env),
            'github.com/idaholab/moose'
        )

        with self.assertRaises(ValueError):
            CIVETStore.get_civet_repo_url({})

    def testGetCIVETServer(self):
        """
        Tests get_civet_server()
        """
        self.assertEqual(
            CIVETStore.get_civet_server(CIVET_SERVER),
            'civet.inl.gov'
        )

    @staticmethod
    def buildHeaderGold(base_sha: str, env: dict) -> dict:
        """
        Build the base gold for build_header() given a
        base sha and an env to read from
        """
        return {
            'civet': {
                'job_id': int(env['CIVET_JOB_ID']),
                'job_url': f'civet.inl.gov/job/{env["CIVET_JOB_ID"]}',
                'recipe_name': env['CIVET_RECIPE_NAME'],
                'repo_url': 'github.com/idaholab/moose',
                'step': int(env['CIVET_STEP_NUM']),
                'step_name': env['CIVET_STEP_NAME']
            },
            'base_sha': base_sha,
            'civet_version': CIVETStore.CIVET_VERSION,
            'event_id': int(env['CIVET_EVENT_ID']),
            'event_sha': env['CIVET_HEAD_SHA'],
        }

    def testBuildHeaderPR(self):
        """
        Tests build_header() for a pull request
        """
        pr_num = 111
        base_sha, base_env = base_civet_env()
        env = {'CIVET_EVENT_CAUSE': 'Pull request', 'CIVET_PR_NUM': str(pr_num)}
        env.update(base_env)

        result = CIVETStore.build_header(base_sha, env)

        gold = self.buildHeaderGold(base_sha, base_env)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/pull/{pr_num}'
        gold['civet']['push_branch'] = env['CIVET_HEAD_REF']
        gold['event_cause'] = 'pr'
        gold['pr_num'] = pr_num
        gold['time'] = result['time']

        self.assertEqual(result, gold)

    @unittest.skipUnless(os.environ.get('CIVET_EVENT_CAUSE') == 'Pull request',
                         f"Skipping because not on a CIVET PR")
    def testBuildHeaderPRLive(self):
        """
        Tests build_header() for a pull request when used on CIVET
        """
        env = dict(os.environ)
        pr_num = int(env['CIVET_PR_NUM'])
        base_sha = env['CIVET_BASE_SHA']

        result = CIVETStore.build_header(base_sha, env)

        gold = self.buildHeaderGold(base_sha, env)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/pull/{pr_num}'
        gold['civet']['push_branch'] = env['CIVET_HEAD_REF']
        gold['event_cause'] = 'pr'
        gold['pr_num'] = pr_num
        gold['time'] = result['time']

        self.assertEqual(result, gold)

    def testBuildHeaderPush(self):
        """
        Tests build_header() for a push event
        """
        base_sha, base_env = base_civet_env()
        env = {'CIVET_EVENT_CAUSE': 'Push next', 'CIVET_PR_NUM': '0'}
        env.update(base_env)

        result = CIVETStore.build_header(base_sha, env)

        gold = self.buildHeaderGold(base_sha, base_env)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/commit/{base_env["CIVET_HEAD_SHA"]}'
        gold['event_cause'] = 'push'
        gold['pr_num'] = None
        gold['time'] = result['time']

        self.assertEqual(result, gold)

    @unittest.skipUnless(os.environ.get('CIVET_EVENT_CAUSE', '').startswith('Push'),
                         "Skipping because not on a CIVET push")
    def testBuildHeaderPushLive(self):
        """
        Tests build_header() for a pull request when used on CIVET
        """
        env = dict(os.environ)
        base_sha = env['CIVET_BASE_SHA']

        result = CIVETStore.build_header(base_sha, env)

        gold = self.buildHeaderGold(base_sha, env)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/commit/{env["CIVET_HEAD_SHA"]}'
        gold['event_cause'] = 'push'
        gold['pr_num'] = None
        gold['time'] = result['time']

        self.assertEqual(result, gold)

    def testBuildHeaderScheduled(self):
        """
        Tests build_header() for a scheduled event
        """
        base_sha, base_env = base_civet_env()
        env = {'CIVET_EVENT_CAUSE': 'Scheduled', 'CIVET_PR_NUM': '0'}
        env.update(base_env)

        result = CIVETStore.build_header(base_sha, env)

        gold = self.buildHeaderGold(base_sha, base_env)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/commit/{base_env["CIVET_HEAD_SHA"]}'
        gold['event_cause'] = 'scheduled'
        gold['pr_num'] = None
        gold['time'] = result['time']

        self.assertEqual(result, gold)

    def testBuildHeaderMissingVariable(self):
        """
        Tests build_header() when a CIVET variable is missing
        """
        base_sha, base_env = base_civet_env()
        env = {'CIVET_EVENT_CAUSE': 'Pull request'}
        env.update(base_env)

        with self.assertRaisesRegex(KeyError, 'Environment variable CIVET_PR_NUM not set'):
            CIVETStore.build_header(base_sha, env)

    def testBuildHeaderBadCause(self):
        """
        Tests build_header() with a bad event cause
        """
        base_sha, base_env = base_civet_env()
        env = {'CIVET_EVENT_CAUSE': 'Foo', 'CIVET_PR_NUM': '0'}
        env.update(base_env)

        with self.assertRaisesRegex(ValueError, 'Unknown event cause "Foo"'):
            CIVETStore.build_header(base_sha, env)

    def checkResult(self, base_sha: str, env: dict, result: dict, stored_results: dict,
                    stored_tests: Optional[dict[TestName, dict]], build_kwargs: dict = {}):
        """
        Combined helper for comparing a test harness result to a set
        of stored results and tests.

        Each test entry is compared directly to the test harness result entry,
        where values are modified/removed as needed based on the arguments
        that are passed to the build method via build_kwargs
        """
        result_id = stored_results.get('_id')
        in_database = result_id is not None

        # Header should exist
        header = CIVETStore.build_header(base_sha, env)
        header['time'] = stored_results['time']
        for key in header:
            self.assertIn(key, stored_results)
            self.assertEqual(stored_results[key], header[key])

        # Check each test
        for test in results_test_iterator(result):
            test_entry = test.value
            name = test.name

            # Ignore skipped tests and this one is skipped
            if build_kwargs.get('ignore_skipped') and \
                test_entry['status']['status'] == 'SKIP':
                self.assertTrue(not results_has_test(stored_results, name))
                continue

            # Start with the original entry and remove/modify
            # the things that would be different
            modified_test_entry = deepcopy(test_entry)

            stored_test_entry = results_test_entry(stored_results, name)
            if stored_tests:
                if in_database:
                    self.assertIsInstance(stored_test_entry, ObjectId)
                    find_test = [doc for doc in stored_tests if doc['_id'] == stored_test_entry]
                    self.assertEqual(len(find_test), 1)
                    stored_test = find_test[0]
                    modified_test_entry['_id'] = stored_test_entry
                    modified_test_entry['result_id'] = result_id
                else:
                    self.assertIsNone(stored_test_entry)
                    stored_test = stored_tests[name]
            else:
                stored_test = stored_test_entry
            self.assertIsInstance(stored_test, dict)

            # Output is removed
            for key in ['output', 'output_files']:
                if key in modified_test_entry:
                    del modified_test_entry[key]
                self.assertNotIn(key, stored_test)

            # Only runtime (runner_run timing entry)
            if build_kwargs.get('only_runtime'):
                modified_test_entry['timing'] = {'runner_run': modified_test_entry['timing']['runner_run']}

            # Removed keys via options
            for key in ['status', 'timing', 'tester']:
                if build_kwargs.get(f'ignore_{key}'):
                    del modified_test_entry[key]

            # Compress the JSON metadata
            tester = modified_test_entry.get('tester', {})
            json_metadata = tester.get('json_metadata', {})
            for k, v in json_metadata.items():
                json_metadata[k] = compress_dict(v)

            self.assertEqual(modified_test_entry, stored_test)

    def runTestBuild(self, run_tests_args: list[str] = DEFAULT_TESTHARNESS_ARGS,
                     run_tests_kwargs: dict = DEFAULT_TESTHARNESS_KWARGS,
                     separate_tests: bool = False,
                     **kwargs):
        """
        Helper for testing the build() method based on a test harness execution.

        The 'build_kwargs' kwarg is passed to the build method and to the check
        method so that things can be checked based on how they are built
        """
        if 'build_kwargs' not in kwargs:
            kwargs['build_kwargs'] = {}
        build_kwargs = kwargs['build_kwargs']

        if separate_tests:
           build_kwargs['max_result_size'] = 1e-6

        base_sha, env = build_civet_env()
        run_tests_result = self.runTestsCached(*run_tests_args, **run_tests_kwargs)
        results = run_tests_result.results

        build_result, build_tests = CIVETStore().build(results,
                                                       base_sha=base_sha,
                                                       env=env,
                                                       **build_kwargs)
        self.assertIsInstance(build_result, dict)
        if separate_tests:
            self.assertIsInstance(build_tests, dict)
            for k, v in build_tests.items():
                self.assertIsInstance(k, TestName)
                self.assertIsInstance(v, dict)
        else:
            self.assertIsNone(build_tests)

        num_tests = len(list(results_test_iterator(build_result)))

        out_regex = rf'Storing {num_tests} tests'
        if build_kwargs.get('ignore_skipped'):
            out_regex += r' \(\d skipped\)'
        out_regex += r' separately' if separate_tests else r' within results'
        out_regex = r'; results size = \d+.\d{2}MB'
        if separate_tests:
            out_regex += r', tests size = \d+.\d{2}MB'

        output = self.stdout_mock.getvalue()
        self.assertRegex(output, out_regex)

        self.checkResult(base_sha, env, results, build_result, build_tests, **kwargs)

    def testBuild(self):
        """
        Test the build() method with no additional arguments and the
        tests contained within the result (result is small enough)
        """
        self.runTestBuild()

    def testBuildSeparateTests(self):
        """
        Test the build() method with no additional arguments and the
        tests stored separate from the results
        """
        self.runTestBuild(separate_tests=True)

    def testBuildIgnoreStatus(self):
        """
        Test the build() method with --ignore-status, not storing
        the status entry
        """
        self.runTestBuild(build_kwargs={'ignore_status': True})

    def testBuildIgnoreTester(self):
        """
        Test the build() method with --ignore-status, not storing
        the tester entry
        """
        self.runTestBuild(build_kwargs={'ignore_tester': True})

    def testBuildIgnoreTiming(self):
        """
        Test the build() method with --ignore-timing, not storing
        the tester entry
        """
        self.runTestBuild(build_kwargs={'ignore_timing': True})

    def testBuildOnlyRuntime(self):
        """
        Test the build() method with --only-runtime, storing
        only the 'runner_run' timing argument
        """
        self.runTestBuild(build_kwargs={'only_runtime': True})

    def testBuildOversizedTest(self):
        """
        Test the build() method throwing an exception when test(s)
        are oversized
        """
        base_sha, env = build_civet_env()
        run_tests_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS, **DEFAULT_TESTHARNESS_KWARGS)
        results = run_tests_result.results

        for test in results_test_iterator(results):
            test.value['foo'] = 'a' * int(MAX_DOCUMENT_SIZE * 1024 * 1024 * 1.5)
            bad_test = test.name
            break

        with self.assertRaises(OversizedTestsError) as context:
            CIVETStore().build(results, base_sha=base_sha, env=env)
        tests = context.exception.tests
        self.assertEqual(len(tests), 1)
        self.assertEqual(tests[0][0], bad_test)
        self.assertGreater(tests[0][1], MAX_DOCUMENT_SIZE)

    def testBuildOversizedResult(self):
        """
        Test the build() method throwing an exception when the
        result is oversized
        """
        base_sha, env = build_civet_env()
        run_tests_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS, **DEFAULT_TESTHARNESS_KWARGS)
        results = run_tests_result.results

        results['foo'] = 'a' * int(MAX_DOCUMENT_SIZE * 1024 * 1024 * 1.5)

        with self.assertRaises(OversizedResultError) as context:
            CIVETStore().build(results, base_sha=base_sha, env=env)
        result_size = context.exception.result_size
        self.assertGreater(result_size, MAX_DOCUMENT_SIZE)

    def testBuildIgnoreSkipped(self):
        """
        Test the build() method with --ignore-skipped, not storing
        tests that are skipped
        """
        run_tests_args = ['-i', 'validation', '--only-tests-that-require', 'libtorch']
        run_tests_kwargs = {'no_capabilities': False}
        self.runTestBuild(run_tests_args=run_tests_args,
                          run_tests_kwargs=run_tests_kwargs,
                          build_kwargs={'ignore_skipped': True})

    def testHasAuth(self):
        """
        Helper for checking if auth is available or not given an environment
        variable. This lets us within CIVET tests assert whether or not
        the authentication is available when we expect it to be so (or not).
        """
        if os.environ.get('TEST_CIVETSTORE_HAS_AUTH') is not None:
            self.assertTrue(HAS_AUTH)
        if os.environ.get('TEST_CIVETSTORE_MISSING_AUTH') is not None:
            self.assertFalse(HAS_AUTH)

    def getStoredResult(self, result_id: ObjectId,
                        test_ids: Optional[list[ObjectId]]) -> Tuple[dict, Optional[list[dict]]]:
        """
        Helper for querying the database for a result and tests
        that have been previously stored
        """
        with CIVETStore.setup_client() as client:
            db = client[TEST_DATABASE]

            result_filter = {'_id': {'$eq': result_id}}
            stored_result = db.results.find_one(result_filter)
            self.assertIsNotNone(stored_result)

            deleted = db.results.delete_one(result_filter)
            self.assertTrue(deleted.acknowledged)
            self.assertEqual(deleted.deleted_count, 1)

            stored_tests = None
            if test_ids:
                test_filter = {'_id': {'$in': test_ids}}
                stored_tests = [doc for doc in db.tests.find({'_id': {'$in': test_ids}})]
                self.assertEqual(len(stored_tests), len(test_ids))

                deleted = db.tests.delete_many(test_filter)
                self.assertTrue(deleted.acknowledged)
                self.assertTrue(deleted.deleted_count, len(test_ids))

        return stored_result, stored_tests

    def runTestStore(self, separate_tests: bool = False, **kwargs):
        """
        Helper for testing the store() method.
        """
        if separate_tests:
            kwargs['max_result_size'] = 1e-6

        base_sha, env = build_civet_env()

        run_tests_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS, **DEFAULT_TESTHARNESS_KWARGS)
        results = run_tests_result.results

        store_args = [TEST_DATABASE, results, base_sha]
        build_kwargs = {'env': env}
        result_id, test_ids = CIVETStore().store(*store_args, **build_kwargs, **kwargs)
        self.assertIsInstance(result_id, ObjectId)
        self.assertIsInstance(test_ids, list if separate_tests else type(None))

        output = self.stdout_mock.getvalue()
        if separate_tests:
            self.assertIn('Inserted 4 tests into ' + TEST_DATABASE, output)
        self.assertIn(f'Inserted result {result_id}', output)

        stored_result, stored_tests = self.getStoredResult(result_id, test_ids)
        self.checkResult(base_sha, env, results, stored_result, stored_tests, build_kwargs=kwargs)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testStore(self):
        """
        Test the store() method with no additional arguments and the
        tests contained within the result (result is small enough)
        """
        self.runTestStore()

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testStoreSeparateTests(self):
        """
        Test the store() method with no additional arguments and the
        tests stored separate from the results
        """
        self.runTestStore(separate_tests=True)

    @unittest.skipUnless(HAS_AUTH, "Skipping because authentication is not available")
    def testMain(self):
        """
        Test running from main
        """
        base_sha, env = build_civet_env()

        run_tests_result = self.runTestsCached(*DEFAULT_TESTHARNESS_ARGS, **DEFAULT_TESTHARNESS_KWARGS)
        result = run_tests_result.results

        with NamedTemporaryFile() as temp_result:
            with open(temp_result.name, 'w') as f:
                json.dump(result, f)

            result_id, test_ids = CIVETStore().main(database=TEST_DATABASE,
                                                     result_path=temp_result.name,
                                                     base_sha=base_sha,
                                                     env=env)

            stored_result, stored_tests = self.getStoredResult(result_id, test_ids)
            self.checkResult(base_sha, env, result, stored_result, stored_tests)

    def testResultFileMissing(self):
        """
        Tests running main() with a results file that doesn't exist
        """
        with self.assertRaisesRegex(SystemExit,
                                    f'Result file {os.path.abspath("foo")} does not exist'):
            CIVETStore().main(database='foo', result_path='foo', out_path='', base_sha='')

if __name__ == '__main__':
    unittest.main()
