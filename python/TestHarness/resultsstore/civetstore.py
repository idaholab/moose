#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import argparse
import json
import os
import re
from copy import deepcopy
from typing import Optional, Tuple
from datetime import datetime
from pymongo import MongoClient
from bson.objectid import ObjectId
from bson import encode

from TestHarness.resultsstore.auth import Authentication, load_authentication, has_authentication
from TestHarness.resultsstore.utils import TestName, compress_dict, results_folder_iterator, \
    results_test_iterator

NoneType = type(None)

# The max size that a result entry can be before its tests are
# stored separately in a tests document
MAX_RESULT_SIZE = 15.0 # MB
# The max size that we'll allow for a document (mongo caps at 16)
MAX_DOCUMENT_SIZE = 15.0 # MB

class OversizedTestsError(SystemExit):
    """
    Exception thrown when test(s) too large for storing.
    """
    def __init__(self, tests: list[Tuple[TestName, float]]):
        message = ['Oversized test(s) were found:']
        message += [f'  {name} ({size:.2f}MB)' for name, size in tests]
        super().__init__('\n'.join(message))

        # The oversized tests (name and size in MB)
        self.tests: list[Tuple[TestName, float]] = tests

class OversizedResultError(SystemExit):
    """
    Exception thrown when a result is too large for storing.
    """
    def __init__(self, result_size: float):
        message = f'Result is oversized ({result_size:.2f}MB)'
        super().__init__(message)

        # The size of the oversized result (MB)
        self.result_size: float = result_size

class CIVETStore:
    """
    Class that handles the storage of TestHarness results
    (loaded from the TestHarness JSON output) to later
    be indexed and retrieved via the ResultsReader, which
    represents tests via StoredResult and StoredTestResult
    objects.

    This class is to be used in practice via command line,
    where one will pass in a JSON output to be stored
    in a database.

    The bulk of the implementation here involves parsing
    the current CIVET environment (mainly from CIVET_*
    environment variables) and storing it in a form
    that can be indexed in a database (for example,
    to load a result given a CIVET event).
    """
    # The current schema version for storage; history:
    # - 6: Initial version when moved to CIVETStore
    CIVET_VERSION = 6

    @staticmethod
    def parse_args() -> argparse.Namespace:
        """
        Parse command-line arguments.
        """
        parser = argparse.ArgumentParser(
            description='Converts test results from a CIVET run for filling into a database'
        )
        parser.add_argument('result_path', type=str, help='Path to the results file')
        parser.add_argument('base_sha', type=str, help='The base commit')
        parser.add_argument('database', type=str, help='The database')
        parser.add_argument('--ignore-skipped', action='store_true', help='Ignore skipped tests')
        parser.add_argument('--ignore-status', action='store_true', help='Ignore status entry in tests')
        parser.add_argument('--ignore-timing', action='store_true', help='Ignore timing entry in tests')
        parser.add_argument('--ignore-tester', action='store_true', help='Ignore tester entry in tests')
        parser.add_argument('--only-runtime', action='store_true', help='Only store runner_run time in tests')
        parser.add_argument('--max-result-size', type=float, default=MAX_RESULT_SIZE,
                            help='Max size of a result for tests to be stored within it')
        return parser.parse_args()

    @staticmethod
    def load_authentication() -> Optional[Authentication]:
        """
        Loads mongo authentication, if available.

        Attempts to first load the authentication environment from
        env vars CIVET_STORE_AUTH_[HOST,USERNAME,PASSWORD] if
        available. Otherwise, tries to load the authentication
        environment from the file set by env var
        CIVET_STORE_AUTH_FILE if it is available.
        """
        return load_authentication('CIVET_STORE')

    @staticmethod
    def has_authentication() -> bool:
        """
        Checks whether or not authentication is available.
        """
        return has_authentication('CIVET_STORE')

    @staticmethod
    def get_document_size(doc: dict) -> float:
        """
        Get the approximate size of a dict if it was
        be stored in a mongodb document in MB.
        """
        assert isinstance(doc, dict)
        return len(encode(doc)) / (1024 * 1024)

    @staticmethod
    def parse_ssh_repo(repo: str) -> Tuple[str, str, str]:
        """
        Parses a Git SSH repo into its server, org, and repo name.

        For example, 'git@github.com:idaholab/moose' ->
        ('github.com', 'idaholab', 'moose').

        Needed because CIVET stores the repository in this form.
        """
        assert isinstance(repo, str)

        search = re.search(r'^git@([a-zA-Z._\-]+):([a-zA-Z0-9._\-]+)\/([a-zA-Z0-9._\-]+).git$',
                            repo)
        if search is None:
            search = re.search(r'^git@([a-zA-Z._\-]+):([a-zA-Z0-9._\-]+)\/([a-zA-Z0-9._\-]+)$',
                                repo)
        if search is None:
            raise ValueError(f'Failed to parse SSH repo from {repo}')
        return search.group(1), search.group(2), search.group(3)

    @staticmethod
    def get_civet_repo_url(env: dict) -> str:
        """
        Determines the URL of the Git repo from the CIVET environment.

        CIVET will set either CIVET_BASE_SSH_URL or APPLICATION_REPO
        to be the Git SSH repository, which are parsed.
        """
        assert isinstance(env, dict)

        ssh_repo = None
        base_ssh_url = env.get('CIVET_BASE_SSH_URL')
        if base_ssh_url is None:
            raise KeyError('Environment variable CIVET_BASE_SSH_URL not set')
        if base_ssh_url:
            ssh_repo = base_ssh_url
        else:
            if (application_repo := env.get('APPLICATION_REPO')):
                ssh_repo = application_repo
            else:
                raise ValueError('Failed to obtain repo from CIVET_BASE_SSH_URL or APPLICATION_REPO')

        server, org, repo = CIVETStore.parse_ssh_repo(ssh_repo)
        return f'{server}/{org}/{repo}'

    @staticmethod
    def get_civet_server(civet_server: str) -> str:
        """
        Parses the CIVET server from the CIVET environment.

        Does some cleanup - changes the backend URL to the
        frontend URL for civet.inl.gov and removes the
        leading 'https://'.
        """
        assert isinstance(civet_server, str)
        # civet.inl.gov reports as civet-be.inl.gov
        civet_server = civet_server.replace('civet-be', 'civet')
        # Remove the https://
        return civet_server.replace('https://', '')

    @staticmethod
    def build_header(base_sha: str, env: dict) -> dict:
        """
        Builds the header for a results entry from the CIVET environment.

        The base event SHA must also be provided because the one that
        is reported from CIVET is not necessarily the real base. This
        can be the case when PRs in MOOSE go into next, but we actually
        base them on devel.

        This data is appended to the main results entry that is
        stored in the database.

        The entries that are stored are:
          - base_sh (str): The base_sha passed to this method
          - civet (dict): A dict containing other CIVET info; this
            is kept separate because it is info that will not be
            indexed from the top level of the database entry
          - civet_version (int): The schema version from this
            storer, set from CIVET_VERSION
          - event_sha (str): The head SHA of the event
          - event_id (int): The ID of the CIVET event
          - event_cause (pr): The cause for this CIVET event;
            current options are ['pr', 'push', 'scheduled']
          - pr_num (int or None): The PR number, if any
          - time (datettime): The current time
        """

        assert isinstance(base_sha, str)
        assert len(base_sha) == 40
        assert isinstance(env, dict)

        # Load variables from the CIVET environment
        civet_env = {}
        load_civet_vars = [
            'event_cause',
            'event_id',
            'head_ref',
            'head_sha',
            'job_id',
            'pr_num',
            'recipe_name',
            'server',
            'step_name',
            'step_num'
        ]
        civet_int_vars = [
            'event_id',
            'job_id',
            'step_num'
        ]
        for var in load_civet_vars:
            civet_var = f'CIVET_{var.upper()}'
            value = env.get(civet_var)
            if value is None:
                raise KeyError(f'Environment variable {civet_var} not set')
            if var in civet_int_vars:
                value = int(value)
            civet_env[var] = value

        assert len(civet_env['head_sha']) == 40

        # Load URL to repo (i.e., github.com/idaholab/moose)
        repo_url = CIVETStore.get_civet_repo_url(env)

        # Load CIVET server (i.e., civet.inl.gov)
        civet_server = CIVETStore.get_civet_server(civet_env['server'])

        # Fill 'civet' entry, which isn't part of the index
        # but adds additional context about the CIVET job
        civet_entry = {
            'job_id': civet_env['job_id'],
            'job_url': f'{civet_server}/job/{civet_env["job_id"]}',
            'recipe_name': civet_env['recipe_name'],
            'repo_url': repo_url,
            'step_name': civet_env['step_name'],
            'step': civet_env['step_num']
        }

        # Get [event_cause, pr_num] for the main header and
        # set [event_url, push_branch] in the civet entry
        event_cause = civet_env['event_cause']
        if event_cause.startswith('Pull'):
            pr_num = int(civet_env['pr_num'])
            event_cause = 'pr'
            civet_entry['event_url'] = f'{repo_url}/pull/{pr_num}'
            civet_entry['push_branch'] = civet_env['head_ref']
        else:
            pr_num = None
            if event_cause.startswith('Push'):
                event_cause = 'push'
            elif event_cause.startswith('Scheduled'):
                event_cause = 'scheduled'
            else:
                raise ValueError(f'Unknown event cause "{event_cause}"')
            civet_entry['event_url'] = f'{repo_url}/commit/{civet_env["head_sha"]}'

        assert isinstance(pr_num, (int, NoneType))

        return {
            'base_sha': base_sha,
            'civet': civet_entry,
            'civet_version': CIVETStore.CIVET_VERSION,
            'event_sha': civet_env['head_sha'],
            'event_id': civet_env['event_id'],
            'event_cause': event_cause,
            'pr_num': pr_num,
            'time': datetime.now()
        }

    def build(self, results: dict, base_sha: str, env: dict = dict(os.environ),
              max_result_size: float = MAX_RESULT_SIZE,
              **kwargs) -> Tuple[dict, Optional[dict[TestName, dict]]]:
        """
        Builds an result entry for storage in the database.

        See the optional parameters for store() for information
        on the keyword arguments that are used here.

        Parameters
        ----------
        results : dict
            The results that come from TestHarness JSON result output.
        base_sha : str
            The base commit SHA for the CIVET event.

        Optional Parameters
        -------------------
        env : dict
            The environment to load the CIVET context from. Defaults
            to the environment from os.environ.
        max_result_size : float
            The max size that a database result entry can have before
            the tests will be stored in a separate 'tests' collection.

        Returns
        -------
        dict:
            The built result data.
        Optional[dict[TestName, dict]]]:
            The separate test data if tests are separate.

        """
        assert isinstance(base_sha, str)
        assert len(base_sha) == 40
        assert isinstance(max_result_size, (float, int))
        assert max_result_size > 0

        results = deepcopy(results)

        version = results['testharness']['version']
        print(f'Loaded results; testharness version = {version}')

        # Append header
        header = self.build_header(base_sha, env)
        results.update(header)

        # Remove skipped tests if requested
        num_skipped_tests = 0
        if kwargs.get('ignore_skipped'):
            for folder in results_folder_iterator(results):
                for test in folder.test_iterator():
                    if test.value['status']['status'] == 'SKIP':
                        num_skipped_tests += 1
                        test.delete()
                folder.delete_if_empty()

        # Cleanup each test as needed
        num_tests = 0
        for test in results_test_iterator(results):
            test_values = test.value
            num_tests += 1

            # Remove all output from results
            for key in ['output', 'output_files']:
                if key in test_values:
                    del test_values[key]

            # Remove keys if requested
            for entry in ['status', 'timing', 'tester']:
                if kwargs.get(f'ignore_{entry}'):
                    del test_values[entry]

            # Only store runner runtime if requested
            if kwargs.get('only_runtime'):
                for key in list(test_values['timing'].keys()):
                    if key != 'runner_run':
                        del test_values['timing'][key]

            # Compress JSON metadata as binary
            tester = test_values.get('tester', {})
            json_metadata = tester.get('json_metadata', {})
            for k, v in json_metadata.items():
                json_metadata[k] = compress_dict(v)

        # Determine the size of the current results dict, which
        # contains the tests within it. If below a certain size,
        # keep the tests within the results data. If above,
        # separate them. We need to do this to keep mongodb
        # documents a reasonable size (the max is 16MB).
        results_size = self.get_document_size(results)
        separate_tests = results_size > max_result_size

        tests = None
        tests_size = None
        oversized_tests = None
        # Separate out the tests if needed
        if separate_tests:
            tests = {}
            tests_size = 0
            oversized_tests = []
            for test in results_test_iterator(results):
                # Test data in the result entry, separated
                test_data = deepcopy(test.value)
                # Store and check test size
                test_size = self.get_document_size(test_data)
                tests_size += test_size
                if test_size > MAX_DOCUMENT_SIZE:
                    oversized_tests.append((test.name, test_size))
                # Store the separate test data
                tests[test.name] = test_data
                # Set the test in the results entry to nothing
                test.set_value(None)

        # Error if tests were too large
        if oversized_tests:
            raise OversizedTestsError(oversized_tests)

        # Recompute the result size if we separated tests
        # as it has changed, and make sure it is small enough
        if separate_tests:
            results_size = self.get_document_size(results)
            if results_size > MAX_DOCUMENT_SIZE:
                raise OversizedResultError(results_size)

        # Share what we're doin
        info = [f'Storing {num_tests} tests']
        if num_skipped_tests:
            info += [f'({num_skipped_tests} skipped)']
        info += [('separately' if separate_tests else 'within results') + ';']
        sizes = [f'results size = {results_size:.2f}MB']
        if separate_tests:
            sizes += [f'tests size = {tests_size:.2f}MB']
        info += [', '.join(sizes)]
        print(' '.join(info))

        return results, tests

    @staticmethod
    def setup_client() -> MongoClient:
        """
        Builds a MongoClient given the available authentication.
        """
        auth = CIVETStore.load_authentication()
        if auth is None:
            raise SystemExit('ERROR: Authentication is not available')
        return MongoClient(auth.host,
                           auth.port,
                           username=auth.username,
                           password=auth.password)

    def store(self, database: str, results: dict,
              base_sha: str, **kwargs) -> Tuple[ObjectId, Optional[list[ObjectId]]]:
        """
        Stores the data in the database from a test harness result.

        Parameters
        ----------
        database : str
            The name of the mongo database to store into
        results : dict
            The results that come from TestHarness JSON result output
        base_sha : str
            The base commit SHA for the CIVET event

        Optional Parameters
        -------------------
        ignore_skipped : bool
            Do not store test entries that have a 'SKIP' status
        ignore_status : bool
            Do not store the 'status' key in test entries
        ignore_timing : bool
            Do not store the 'timing' key in test entries
        ignore_tester : bool
            Do not store the 'test' key in test entries
        only_runtime : bool
            Only store the 'runner_run' key in test 'timing' entries

        Returns
        -------
        ObjectID:
            The mongo ObjectID of the inserted results document
        list[ObjectId] or None:
            The mongo ObjectIDs of the inserted test documents, if any;
            this will be None if tests are small enough to be stored
            within the results document (determined by build())
        """
        assert isinstance(database, str)

        result, tests = self.build(results, base_sha, **kwargs)

        # Assign an ID for the result
        result_id = ObjectId()
        result['_id'] = result_id

        # Build the tests to be stored separately, if any
        insert_tests = []
        test_ids = None
        if tests:
            test_ids = []
            for result_test in results_test_iterator(result):
                # Get the separate test data
                test_data = tests[result_test.name]

                # Assign an ID to the test
                test_id = ObjectId()
                test_ids.append(test_id)
                test_data['_id'] = test_id
                test_data['result_id'] = result_id

                # Add to be inserted later
                insert_tests.append(test_data)

                # Set the ID in the results for this test
                # to this test's ID
                assert result_test.value is None
                result_test.set_value(test_id)

        # Do the insertion
        with self.setup_client() as client:
            db = client[database]

            inserted_result = db.results.insert_one(result)
            assert inserted_result.acknowledged
            assert result_id == inserted_result.inserted_id
            print(f'Inserted result {result_id} into {database}')

            if insert_tests:
                inserted_tests = db.tests.insert_many(insert_tests)
                assert inserted_tests.acknowledged
                assert set(test_ids) == set(inserted_tests.inserted_ids)
                print(f'Inserted {len(test_ids)} tests into {database}')

        return result_id, test_ids

    def main(self, result_path: str, database: str, base_sha: str,
             **kwargs) -> Tuple[ObjectId, Optional[list[ObjectId]]]:
        """
        Main method that is called when executed from command line.

        See the store() method for common optional keyword args.

        Parameters
        ----------
        result_path : str
            The path to the TestHarness JSON results file
        database : str
            The name of the mongo database to store into
        base_sha : str
            The base commit SHA for the CIVET event

        Returns
        -------
        ObjectID:
            The mongo ObjectID of the inserted results document
        list[ObjectId] or None:
            The mongo ObjectIDs of the inserted test documents, if any;
            this will be None if tests are small enough to be stored
            within the results document (determined by build())
        """
        assert isinstance(result_path, str)
        assert isinstance(database, str)
        assert isinstance(base_sha, str)

        result_path = os.path.abspath(result_path)
        if not os.path.isfile(result_path):
            raise SystemExit(f'Result file {result_path} does not exist')
        with open(result_path, 'r') as f:
            results = json.load(f)

        return self.store(database, results, base_sha, **kwargs)

if __name__ == '__main__':
    args = CIVETStore.parse_args()
    CIVETStore().main(**vars(args))
