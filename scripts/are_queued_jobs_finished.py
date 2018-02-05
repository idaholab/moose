#!/usr/bin/env python
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os, json, re, subprocess
from tempfile import TemporaryFile

class Jobs:
    """ Class to manage I/O to the supplied json file """
    def __init__(self, json_file):
        if os.path.exists(json_file):
            self.__json_file = json_file
        else:
            raise Exception('File does not exist: %s' % (json_file))
        self.__job_data = None

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
        if 'job_id' in meta and meta['status_bucket']['status'] == 'QUEUED':
            job_ids.append(meta['job_id'])
    return job_ids

def getJobStatus(job_ids):
    """ run qstat and return tuple of (job_id, status) """
    results = ''
    qstat_cmd = ['qstat', '-xf']
    qstat_cmd.extend(job_ids)
    t = TemporaryFile()
    process = subprocess.Popen(qstat_cmd, stdout=t)
    process.wait()
    if process.returncode != 0:
        sys.exit(1)
    else:
        t.seek(0)
        results = t.read()

    t.close()
    return results

def updateStatus(jobs, finished_jobs):
    """ loop through jobs and adjust the status for those supplied in finished_jobs """
    for job, meta in jobs.getJobs().iteritems():
        # Use list splicing to simplify the search process
        if 'job_id' in meta \
           and meta['job_id'] in finished_jobs[0::2] \
           and meta['status_bucket']['status'] == 'QUEUED':

            my_status = finished_jobs[finished_jobs.index(meta['job_id']) + 1]

            if my_status['status'] == 'FAIL':
                meta['caveat_message'] = 'Killed by PBS'
            meta['status_bucket'] = my_status

def isFinished(jobs):
    """ Update job statuses, and return bool if all jobs are finished. """
    is_finished = True
    previously_queued_ids = getQueuedJobIDs(jobs)
    if previously_queued_ids:
        current_statuses = getJobStatus(previously_queued_ids)
        finished_jobs = []
        for job in current_statuses.split('\n\n'):
            if job:
                job_id = re.search(r'^Job Id: (\d+)\.[\W\w]+$', job).group(1)
                job_state = re.search(r'job_state = (\w)', job).group(1)
                if job_state == 'F':
                    job_status = {'status' :'WAITING', 'color' : 'CYAN' }

                    # if exit status is non-zero, fail the job
                    if int(re.search(r'Exit_status = (\d+)', job).group(1)):
                        job_status = { 'status' :'FAIL', 'color' : 'RED' }

                    finished_jobs.extend([job_id, job_status])
                else:
                    is_finished = False

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
