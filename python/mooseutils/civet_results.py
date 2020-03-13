#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import re
import glob
import tarfile
if sys.version_info[0] == 3:
    import enum
    import urllib.request
    import urllib.error
else:
    import urllib2
import collections
import logging

DEFAULT_JOBS_CACHE = os.path.join(os.getenv('HOME'), '.local', 'share', 'civet', 'jobs')
DEFAULT_CIVET_SITE = 'https://civet.inl.gov'
DEFAULT_CIVET_REPO = 'idaholab/moose'

TEST_RE = re.compile(r'^(?:\[(?P<time>.+?)s\])?'       # Optional test time
                     r' *(?P<status>[A-Z]+?)'          # Test status (e.g., OK)
                     r' +(?P<test>.*?)'                # Test name
                     r'(?: +(?P<reason>.*?))?'         # reason FAILED (FAILED (ERRORMSG))
                     r'(?: *\[(?P<caveats>.*?)\])?$',  # Test caveats (e.g., [min_cpus=1])
                     flags=re.MULTILINE)

JOB_RE = re.compile(r'id=\"job_(?P<job>\d+)\"')
RECIPE_RE = re.compile(r'results_(?P<number>\d+)_(?P<job>.*)/(?P<recipe>.*)')
RUN_TESTS_START_RE = re.compile(r'^.+?run_tests.+?$', flags=re.MULTILINE)
RUN_TESTS_END_RE = re.compile(r'^-{5,}$', flags=re.MULTILINE)
RESULT_FILENAME_RE = re.compile(r'results_(?P<number>[0-9]+)_(?P<recipe>.*)\.tar\.gz')

Test = collections.namedtuple('Test', 'recipe status caveats reason time url')
Job = collections.namedtuple('Job', 'number filename status url')

if sys.version_info[0] == 3:
    class JobFileStatus(enum.Enum):
        """Status flag for Job file downloads"""
        CACHE = 0
        LOCAL = 1
        DOWNLOAD = 2
        FAIL = 3
else:
    class JobFileStatus(object):
        """Status flag for Job file downloads"""
        CACHE = 0
        LOCAL = 1
        DOWNLOAD = 2
        FAIL = 3


def _get_local_civet_jobs(location, logger=None):
    """
    Get a list of Job objects for the supplied directory; this searches for tar.gz files with the
    name as: results_<JOB>_*.tar.gz.
    """
    jobs = set()
    for filename in glob.glob(os.path.join(location, 'results_*.tar.gz')):
        match = RESULT_FILENAME_RE.search(filename)
        if match:
            jobs.add(Job(int(match.group('number')), filename, JobFileStatus.LOCAL, None))
    return sorted(jobs, key=lambda j: j.number)

def _get_remote_civet_jobs(hashes, site, repo, cache=DEFAULT_JOBS_CACHE, logger=None):
    """
    Get a list of Job objects for the supplied git SHA1 strings.
    """

    jobs = set()
    for sha1 in hashes:
        url = '{}/sha_events/{}/{}'.format(site, repo, sha1)

        if sys.version_info[0] == 3:
            pid = urllib.request.urlopen(url)
        else:
            pid = urllib2.urlopen(url)

        page = pid.read().decode('utf8')
        for match in JOB_RE.finditer(page):
            job = jobs.add(_download_job(int(match.group('job')), site, cache, logger))

    return sorted(jobs, key=lambda j: j.number)

def _download_job(job, site, cache, logger):
    """
    Download, if it doesn't already exist, the raw data file from CIVET testing given a Job object.
    """

    if not os.path.isdir(cache):
        os.makedirs(cache)

    url = '{}/job_results/{}'.format(site, job)
    filename = '{}/results_{}.tar.gz'.format(cache, job)

    status = None
    if os.path.isfile(filename):
        if logger:
            logger.debug('Using cached results for job %s', job)
        status = JobFileStatus.CACHE

    else:
        try:
            if sys.version_info[0] == 3:
                response = urllib.request.urlopen(url)
            else:
                response = urllib2.urlopen(url)

            if response.code == 200:
                if logger:
                    logger.debug('Downloading results for job %s', job)
                with open(filename, 'wb') as fid:
                    fid.write(response.read())
                status = JobFileStatus.DOWNLOAD

        except urllib.error.HTTPError:
            if logger:
                logger.warning('Failed to download results for job %s: %s', job, url)
            status = JobFileStatus.FAIL

    return Job(job, filename, status, site)

def _update_database_from_job(job, database, possible):
    """
    Update the test result database given a Job object.
    """

    if job.status == JobFileStatus.FAIL:
        return

    tar = tarfile.open(job.filename, 'r:gz')
    for member in tar.getmembers():
        match = RECIPE_RE.search(member.name)
        recipe = match.group('recipe')
        number = int(match.group('number'))

        f = tar.extractfile(member)
        if f is not None:
            content = f.read().decode('utf8')
            begin = RUN_TESTS_START_RE.search(content)
            if begin is None:
                continue
            end = RUN_TESTS_END_RE.search(content)
            if end is not None:
                _process_results(database, job, recipe, content[begin.end():end.start()], possible)

def _process_results(database, job, recipe, content, possible):
    """
    Extract results from run_tests and update the database.
    """
    for match in TEST_RE.finditer(content):
        caveats = match.group('caveats')
        reason = match.group('reason')
        if caveats is not None:
            caveats = caveats.split(',')

        time = match.group('time')
        if time is not None:
            time = float(time)

        tname = match.group('test').split(':')[-1]
        status = match.group('status')
        if (possible is None) or (status in possible):
            url = job.url if job.url is not None else job.filename
            database[tname][job.number].append(Test(recipe, status, caveats, reason, time, url))

def get_civet_results(local=list(),
                      hashes=list(),
                      sites=[(DEFAULT_CIVET_SITE, DEFAULT_CIVET_REPO)],
                      possible=None,
                      cache=DEFAULT_JOBS_CACHE, logger=None):

    database = collections.defaultdict(lambda: collections.defaultdict(list))
    for loc in local:
        jobs = _get_local_civet_jobs(loc, logger=logger)
        for job in jobs:
            _update_database_from_job(job, database, possible)

    for site, repo in sites:
        jobs = _get_remote_civet_jobs(hashes, site, repo, cache=cache, logger=logger)
        for job in jobs:
            _update_database_from_job(job, database, possible)
    return database

if __name__ == '__main__':
    #database = get_civet_results(hashes=['681ba2f4274dc8465bb2a54e1353cfa24765a5c1',
    #                                    'febe3476040fe6af1df1d67e8cc8c04c4760afb6'])
    database = get_civet_results(sites=[],
                                 local=['/Users/slauae/projects/moose/python/MooseDocs/test/content/civet'])
