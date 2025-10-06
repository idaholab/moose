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
import sys
import re
from copy import deepcopy
from typing import Optional, Tuple
from datetime import datetime
from pymongo import MongoClient
from bson.objectid import ObjectId
from bson.binary import Binary
import zlib

from TestHarness.resultsreader.auth import Authentication, load_authentication, has_authentication

NoneType = type(None)

# The max size that a result entry can be before its tests are
# stored separately in a tests document
MAX_RESULT_SIZE = 5.0 # MB

class CIVETStorer:
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
        env vars CIVET_STORER_AUTH_[HOST,USERNAME,PASSWORD] if
        available. Otherwise, tries to load the authentication
        environment from the file set by env var
        CIVET_STORER_AUTH_FILE if it is available.
        """
        return load_authentication('CIVET_STORER')

    @staticmethod
    def has_authentication() -> bool:
        """
        Checks whether or not authentication is available.
        """
        return has_authentication('CIVET_STORER')

    @staticmethod
    def get_size(obj, seen: Optional[set] = None) -> int:
        """
        Recursively find the size of an object in bytes.
        """
        get_size = CIVETStorer.get_size
        size = sys.getsizeof(obj)
        if seen is None:
            seen = set()
        obj_id = id(obj)
        if obj_id in seen:
            return 0
        seen.add(obj_id)
        if isinstance(obj, dict):
            size += sum([get_size(v, seen) for v in obj.values()])
            size += sum([get_size(k, seen) for k in obj.keys()])
        elif hasattr(obj, '__dict__'):
            size += get_size(obj.__dict__, seen)
        elif hasattr(obj, '__iter__') and not isinstance(obj, (str, bytes, bytearray)):
            size += sum([get_size(i, seen) for i in obj])
        return size

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
            raise ValueError(f'Failed to parse SSH repo from {"value"}')
        return search.group(1), search.group(2), search.group(3)

    @staticmethod
    def get_civet_repo_url(env: dict) -> str:
        """
        Determines the URL of the Git repo from the CIVET environment.

        CIVET will set either CIVET_BASE_SSH_URL or APPLICATION_REPO
        to be the Git SSH repository, which are parsed.
        """
        assert isinstance(env, dict)

        ssh_repo = env.get('CIVET_BASE_SSH_URL', env.get('APPLICATION_REPO'))
        if ssh_repo is None:
            raise ValueError('Failed to obtain repo from CIVET_BASE_SSH_URL or APPLICATION_REPO')

        server, org, repo = CIVETStorer.parse_ssh_repo(ssh_repo)
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
        repo_url = CIVETStorer.get_civet_repo_url(env)

        # Load CIVET server (i.e., civet.inl.gov)
        civet_server = CIVETStorer.get_civet_server(civet_env['server'])

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
            'civet_version': CIVETStorer.CIVET_VERSION,
            'event_sha': civet_env['head_sha'],
            'event_id': civet_env['event_id'],
            'event_cause': event_cause,
            'pr_num': pr_num,
            'time': datetime.now()
        }

    def build(self, results: dict, base_sha: str, env: dict,
              max_result_size: float = MAX_RESULT_SIZE, **kwargs) -> Tuple[dict, Optional[list]]:
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
        env : dict
            The environment to load the CIVET context from.

        Optional Parameters
        -------------------
        max_result_size : float
            The max size that a database result entry can have before
            the tests will be stored in a separate 'tests' collection.
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

        tests_entry = results['tests']

        # Remove skipped tests if requested
        skip_tests = []
        if kwargs.get('ignore_skipped'):
            for folder_name, folder_values in tests_entry.items():
                for test_name, test_values in folder_values['tests'].items():
                    if test_values['status']['status'] == 'SKIP':
                        skip_tests.append((folder_name, test_name))
            for folder_name, test_name in skip_tests:
                del tests_entry[folder_name]['tests'][test_name]
                if not tests_entry[folder_name]['tests']:
                    del tests_entry[folder_name]

        # Cleanup each test as needed
        for folder_values in tests_entry.values():
            for test_values in folder_values['tests'].values():
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

                # Append metadata content from file
                json_metadata = test_values.get('tester', {}).get('json_metadata', {})
                for key, value in json_metadata.items():
                    if value:
                        try:
                            with open(value, 'r') as f:
                                json_metadata[key] = json.load(f)
                        except:
                            print(f'WARNING: Failed to load metadata file {value}')

        tests = None
        max_result_size = max_result_size * 1e6
        results_size = self.get_size(results)
        results_size_mb = results_size / 1e6
        if results_size < max_result_size:
            print(f'Storing tests within results; size = {results_size_mb:.2f}MB')
        else:
            print(f'Storing tests separately; size = {results_size_mb:.2f}MB')
            i = 0
            tests = []
            for folder_values in tests_entry.values():
                folder_tests = folder_values['tests']
                for test_name in list(folder_tests.keys()):
                    tests.append(deepcopy(folder_tests[test_name]))
                    folder_tests[test_name] = i
                    i += 1

        return results, tests

    @staticmethod
    def setup_client() -> MongoClient:
        """
        Builds a MongoClient given the available authentication.
        """
        auth = CIVETStorer.load_authentication()
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

        auth = self.load_authentication()
        if auth is None:
            raise SystemExit('ERROR: Authentication is not available')
        client_args = [auth.host, auth.port]
        client_kwargs = {'username': auth.username, 'password': auth.password}
        with self.setup_client() as client:
            db = client[database]

            insert_tests = []
            insert_test_names = []
            for folder_name, folder_entry in result['tests'].items():
                for test_name, test_entry in folder_entry['tests'].items():
                    if tests:
                        test_data = tests[test_entry]
                        test_data['result_id'] = None
                        insert_tests.append(test_data)
                        insert_test_names.append((folder_name, test_name))
                    else:
                        test_data = test_entry

                    # Store larger contents as binary
                    tester = test_data.get('tester', {})
                    json_metadata = tester.get('json_metadata', {})
                    for k, v in json_metadata.items():
                        if v:
                            compressed = Binary(zlib.compress(json.dumps(v).encode('utf-8')))
                            json_metadata[k] = compressed

            # Store the tests (if any)
            test_ids = None
            if insert_tests:
                assert tests is not None
                inserted = db.tests.insert_many(insert_tests)
                assert inserted.acknowledged
                test_ids = inserted.inserted_ids
                print(f'Inserted {len(test_ids)} tests into {database}')

                # Update the test IDs for each test in the result
                for i, id in enumerate(test_ids):
                    folder_name, test_name = insert_test_names[i]
                    result['tests'][folder_name]['tests'][test_name] = id

            # Store the result
            inserted = db.results.insert_one(result)
            assert inserted.acknowledged
            result_id = inserted.inserted_id
            print(f'Inserted result {result_id} into {database}')

            # Update the result ID for each test
            if test_ids:
                filter = {'_id': {'$in': test_ids}}
                update = {'$set': {'result_id': result_id}}
                updated = db.tests.update_many(filter, update)
                assert updated.acknowledged
                assert updated.modified_count == len(test_ids)
                print('Updated test result IDs')

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
    args = CIVETStorer.parse_args()
    CIVETStorer().main(**vars(args))
