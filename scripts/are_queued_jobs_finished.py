#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os, json, re, subprocess

class Jobs:
    """ Class to manage I/O to the supplied json file """
    def __init__(self, json_file):
        if os.path.exists(json_file):
            self.__json_file = json_file
        else:
            raise Exception('File does not exist: %s' % (json_file))
        self.__job_data = None
        self._make_compatible()

    def _make_compatible(self):
        self.getJobs()
        if self.__job_data:
            for job, meta in self.__job_data.iteritems():
                if 'job_id' in meta and not 'are_my_queued_jobs_finished' in meta:
                    meta['are_my_queued_jobs_finished'] = False

    def getJobs(self):
        if self.__job_data is None:
            with open(self.__json_file, 'r') as f:
                try:
                    job_data = json.load(f)
                except ValueError:
                    raise Exception('Not a json file')

            self.__job_data = job_data
        return self.__job_data

    def saveJobs(self):
        if self.getJobs():
            with open(self.__json_file, 'w') as f:
                json.dump(self.__job_data, f, indent=2)

def getQueuedJobIDs(jobs):
    """ Return a list of queued job ids contained in jobs """
    job_ids = []
    for job, meta in jobs.getJobs().iteritems():
        if 'job_id' in meta and meta['are_my_queued_jobs_finished'] == False:
            job_ids.append(meta['job_id'])
    return job_ids

def getJobStatus(job_ids):
    """ run qstat and return tuple of (job_id, status) """
    qstat_cmd = ['qstat', '-x']
    qstat_cmd.extend(job_ids)
    process = subprocess.Popen(qstat_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    results = process.communicate()
    if process.returncode != 0:
        sys.exit(1)
    else:
        parsed_statuses = re.findall(r'(\d+).*\d+ (\w)', results[0])
        return parsed_statuses

def updateStatus(jobs, finished_jobs):
    """ loop through jobs and adjust the status for those supplied in finished_jobs """
    for job, meta in jobs.getJobs().iteritems():
        if 'job_id' in meta and meta['job_id'] in finished_jobs and meta['are_my_queued_jobs_finished'] == False:
            meta['are_my_queued_jobs_finished'] = True

def isFinished(jobs):
    """ Update job statuses, and return bool if all jobs are finished. """
    is_finished = True
    previously_queued_ids = getQueuedJobIDs(jobs)
    if previously_queued_ids:
        current_statuses = getJobStatus(previously_queued_ids)
        finished_jobs = []
        for job_id, status in current_statuses:
            if status != 'F':
                is_finished = False
                continue
            finished_jobs.append(job_id)
        updateStatus(jobs, finished_jobs)

    jobs.saveJobs()
    return is_finished

def usage():
    print('Supply a path to json queue file. Multiple files are supported, in which case all'
          'tests in all json files must be finished for this script to exit with 0.')
    sys.exit(1)

if __name__ == '__main__':
    args = sys.argv[1:]
    if len(args) == 0:
        usage()

    for queue_file in args:
        jobs = Jobs(queue_file)
        if not isFinished(jobs):
            sys.exit(1)
