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
from copy import deepcopy
from typing import Optional, Tuple
from bson.objectid import ObjectId

from TestHarness.resultsreader.civetstorer import CIVETStorer
from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase

# Whether or not authentication is available from env var RESULTS_READER_AUTH_FILE
HAS_AUTH = CIVETStorer.has_authentication()

# Name for the database used for testing database store
TEST_DATABASE = 'civet_tests_moose_resultsreader'

# Dummy sha to use for the base commit
BASE_SHA = '3b491e178466a1ec9d627d3009fbd89ea02b4779'
# Dummy civet environment
BASE_CIVET_ENV = {
    'APPLICATION_REPO': 'git@github.com:idaholab/moose',
    'CIVET_BASE_SHA': 'cf08101a177b17939f4d06f0a5e0a6a8dc2fcadd',
    'CIVET_BASE_SSH_URL': 'git@github.com:idaholab/moose.git',
    'CIVET_EVENT_ID': '12345',
    'CIVET_HEAD_REF': 'branchname',
    'CIVET_HEAD_SHA': '10b839a32d10e80041ea5dd93d976d92319339c6',
    'CIVET_JOB_ID': '567',
    'CIVET_RECIPE_NAME': 'Awesome recipe',
    'CIVET_SERVER': 'https://civet-be.inl.gov',
    'CIVET_STEP_NAME': 'Cool step',
    'CIVET_STEP_NUM': '4'
}

# Base civet environment to be used for testing build() and store()
BUILD_CIVET_ENV = {
    'CIVET_EVENT_CAUSE': 'Pull request',
    'CIVET_PR_NUM': '1234'
}
BUILD_CIVET_ENV.update(BASE_CIVET_ENV)

class TestResultsReaderCIVETStorer(TestHarnessTestCase):
    def testParseSSHRepo(self):
        """
        Tests parse_ssh_repo()
        """
        self.assertEqual(
            CIVETStorer.parse_ssh_repo(BASE_CIVET_ENV['APPLICATION_REPO']),
            ('github.com', 'idaholab', 'moose')
        )

        self.assertEqual(
            CIVETStorer.parse_ssh_repo('git@othergithub.host.com:org/repo.git'),
            ('othergithub.host.com', 'org', 'repo')
        )

        with self.assertRaises(ValueError):
            CIVETStorer.parse_ssh_repo('foo')

    def testGetRepoURL(self):
        """
        Tests get_repo_url()
        """
        env = {'CIVET_BASE_SSH_URL': BASE_CIVET_ENV['APPLICATION_REPO']}
        self.assertEqual(
            CIVETStorer.get_repo_url(env),
            'github.com/idaholab/moose'
        )

        env = {'APPLICATION_REPO': BASE_CIVET_ENV['APPLICATION_REPO']}
        self.assertEqual(
            CIVETStorer.get_repo_url(env),
            'github.com/idaholab/moose'
        )

        with self.assertRaises(ValueError):
            CIVETStorer.get_repo_url({})

    def testGetCIVETServer(self):
        """
        Tests get_civet_server()
        """
        self.assertEqual(
            CIVETStorer.get_civet_server(BASE_CIVET_ENV['CIVET_SERVER']),
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
            'civet_version': CIVETStorer.CIVET_VERSION,
            'event_id': int(env['CIVET_EVENT_ID']),
            'event_sha': env['CIVET_HEAD_SHA'],
        }

    def testBuildHeaderPR(self):
        """
        Tests build_header() for a pull request
        """
        pr_num = 111
        env = {'CIVET_EVENT_CAUSE': 'Pull request', 'CIVET_PR_NUM': str(pr_num)}
        env.update(BASE_CIVET_ENV)

        result = CIVETStorer.build_header(BASE_SHA, env)

        gold = self.buildHeaderGold(BASE_SHA, BASE_CIVET_ENV)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/pull/{pr_num}'
        gold['civet']['push_branch'] = BASE_CIVET_ENV['CIVET_HEAD_REF']
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

        result = CIVETStorer.build_header(base_sha, env)

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
        env = {'CIVET_EVENT_CAUSE': 'Push next', 'CIVET_PR_NUM': '0'}
        env.update(BASE_CIVET_ENV)

        result = CIVETStorer.build_header(BASE_SHA, env)

        gold = self.buildHeaderGold(BASE_SHA, BASE_CIVET_ENV)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/commit/{BASE_CIVET_ENV["CIVET_HEAD_SHA"]}'
        gold['event_cause'] = 'push'
        gold['pr_num'] = None
        gold['time'] = result['time']

        self.assertEqual(result, gold)

    @unittest.skipUnless(os.environ.get('CIVET_EVENT_CAUSE', '').startswith('Push'),
                         f"Skipping because not on a CIVET push")
    def testBuildHeaderPushLive(self):
        """
        Tests build_header() for a pull request when used on CIVET
        """
        env = dict(os.environ)
        base_sha = env['CIVET_BASE_SHA']

        result = CIVETStorer.build_header(base_sha, env)

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
        env = {'CIVET_EVENT_CAUSE': 'Scheduled', 'CIVET_PR_NUM': '0'}
        env.update(BASE_CIVET_ENV)

        result = CIVETStorer.build_header(BASE_SHA, env)

        gold = self.buildHeaderGold(BASE_SHA, BASE_CIVET_ENV)
        gold['civet']['event_url'] = f'github.com/idaholab/moose/commit/{BASE_CIVET_ENV["CIVET_HEAD_SHA"]}'
        gold['event_cause'] = 'scheduled'
        gold['pr_num'] = None
        gold['time'] = result['time']

        self.assertEqual(result, gold)

    def testBuildHeaderMissingVariable(self):
        """
        Tests build_header() when a CIVET variable is missing
        """
        env = {'CIVET_EVENT_CAUSE': 'Pull request'}
        env.update(BASE_CIVET_ENV)

        with self.assertRaisesRegex(KeyError, 'Environment variable CIVET_PR_NUM not set'):
            CIVETStorer.build_header(BASE_SHA, env)

    def testBuildHeaderBadCause(self):
        """
        Tests build_header() with a bad event cause
        """
        env = {'CIVET_EVENT_CAUSE': 'Foo', 'CIVET_PR_NUM': '0'}
        env.update(BASE_CIVET_ENV)

        with self.assertRaisesRegex(ValueError, 'Unknown event cause "Foo"'):
            CIVETStorer.build_header(BASE_SHA, env)

    def checkResult(self, result: dict, stored_results: dict,
                    stored_tests: Optional[list], store_kwargs: dict = {}):
        """
        Helper for checking a result and tests based on
        a test harness result
        """
        result_id = stored_results.get('_id')
        in_database = result_id is not None

        # Header should exist
        header = CIVETStorer.build_header(BASE_SHA, BUILD_CIVET_ENV)
        header['time'] = stored_results['time']
        for key in header:
            self.assertIn(key, stored_results)
            self.assertEqual(stored_results[key], header[key])

        # Check each test
        for folder_name, folder_entry in result['tests'].items():
            for test_name, test_entry in folder_entry['tests'].items():
                # Start with the original entry and remove/modify
                # the things that would be different
                modified_test_entry = deepcopy(test_entry)

                if stored_tests:
                    stored_test_id = stored_results['tests'][folder_name]['tests'][test_name]
                    if in_database:
                        self.assertIsInstance(stored_test_id, ObjectId)
                        find_test = [doc for doc in stored_tests if doc['_id'] == stored_test_id]
                        self.assertEqual(len(find_test), 1)
                        stored_test = find_test[0]
                        modified_test_entry['_id'] = stored_test_id
                        modified_test_entry['result_id'] = result_id
                    else:
                        self.assertIsInstance(stored_test_id, int)
                        stored_test = stored_tests[stored_test_id]
                    pass
                else:
                    stored_test = stored_results['tests'][folder_name]['tests'][test_name]
                self.assertIsInstance(stored_test, dict)

                # Output is removed
                for key in ['output', 'output_files']:
                    if key in modified_test_entry:
                        del modified_test_entry[key]
                    self.assertNotIn(key, stored_test)

                # JSON metadata is loaded from file so it'll differ
                if 'tester' in stored_test:
                    modified_test_entry['tester']['json_metadata'] = stored_test['tester']['json_metadata']

                # Only runtime (runner_run timing entry)
                if store_kwargs.get('only_runtime'):
                    modified_test_entry['timing'] = {'runner_run': modified_test_entry['timing']['runner_run']}

                # Removed keys via options
                for key in ['status', 'timing', 'tester']:
                    if store_kwargs.get(f'ignore_{key}'):
                        del modified_test_entry[key]

                self.assertEqual(modified_test_entry, stored_test)

    def testBuild(self):
        """
        Test the build() method with the various options
        """
        run_tests_result = self.runTests('-i', 'validation', '--capture-perf-graph', exit_code=132)

        def run_test(**kwargs):
            result_copy = deepcopy(run_tests_result.results)
            stored_result, stored_tests = CIVETStorer().build(
                result_copy,
                base_sha=BASE_SHA,
                env=BUILD_CIVET_ENV,
                **kwargs.get('store_kwargs', {})
            )
            self.checkResult(result_copy, stored_result, stored_tests, **kwargs)

        # Store when tests are contained within the result
        run_test()

        # Store when tests are not contained within the result,
        # modified by setting the max result size to something small
        run_test(store_kwargs={'max_result_size': 1e-6})

        # Store with --ignore-[status, tester, timing], not storing those test keys
        for key in ['status', 'tester', 'timing']:
            run_test(store_kwargs={f'ignore_{key}': True})

        # Only store runtime
        run_test(store_kwargs={'only_runtime': True})

    def getStoredResult(self, result_id: ObjectId,
                        test_ids: Optional[list[ObjectId]]) -> Tuple[dict, Optional[list[dict]]]:
        """
        Helper for querying the database for a result and tests
        that have been previously stored
        """
        with CIVETStorer.setup_client() as client:
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

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testStore(self):
        """
        Test the store() method with the various options
        """
        run_tests_result = self.runTests('-i', 'validation', '--capture-perf-graph', exit_code=132)

        def run_test(separate_tests=False, **kwargs):
            result_copy = deepcopy(run_tests_result.results)

            store_args = [TEST_DATABASE, result_copy, BASE_SHA]
            store_kwargs = {'env': BUILD_CIVET_ENV}
            result_id, test_ids = CIVETStorer().store(*store_args, **store_kwargs, **kwargs)
            self.assertIsInstance(result_id, ObjectId)
            self.assertIsInstance(test_ids, (list, type(None)))

            stored_result, stored_tests = self.getStoredResult(result_id, test_ids)
            self.checkResult(result_copy, stored_result, stored_tests, store_kwargs=kwargs)

        # Store with tests combined with results
        run_test()

        # Store with tests separate from results
        run_test(max_result_size=1e-6)

    @unittest.skipUnless(HAS_AUTH, f"Skipping because authentication is not available")
    def testMain(self):
        """
        Test running from main
        """
        run_tests_result = self.runTests('-i', 'validation', '--capture-perf-graph', exit_code=132)
        result = run_tests_result.results

        with NamedTemporaryFile() as temp_result:
            with open(temp_result.name, 'w') as f:
                json.dump(result, f)

            result_id, test_ids = CIVETStorer().main(database=TEST_DATABASE,
                                                     result_path=temp_result.name,
                                                     base_sha=BASE_SHA,
                                                     env=BUILD_CIVET_ENV)

            stored_result, stored_tests = self.getStoredResult(result_id, test_ids)
            self.checkResult(result, stored_result, stored_tests)

    def testResultFileMissing(self):
        """
        Tests running main() with a results file that doesn't exist
        """
        with self.assertRaisesRegex(SystemExit,
                                    f'Result file {os.path.abspath("foo")} does not exist'):
            CIVETStorer().main(database='foo', result_path='foo', out_path='', base_sha='')

if __name__ == '__main__':
    unittest.main()
