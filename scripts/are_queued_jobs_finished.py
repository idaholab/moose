#!/usr/bin/env python
import sys, os, json, re, subprocess

def getJobStatus(job_ids):
    """ run qstat and obtian job status for supplied job_ids """
    qstat_cmd = ['qstat', '-x']
    qstat_cmd.extend(job_ids)
    process = subprocess.Popen(qstat_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    results = process.communicate()
    if process.returncode != 0:
        sys.exit(1)
    else:
        return results[0]

def isFinished(queue_file, job_ids):
    """
    itemize qstat output, and return false if there are any
    jobs that have yet to finish.
    """
    job_results = getJobStatus(job_ids)
    result_list = re.findall(r'(\d+).*\d+ (\w)', job_results)
    finished_job_ids = []
    not_finished = False
    for job_id, status in result_list:
        if status != 'F':
            not_finished = True
            continue
        finished_job_ids.append(job_id)

    # Update the json file with any finished statuses
    saveStatus(queue_file, finished_job_ids)

    if not_finished:
        return False
    return True

def getJobIDs(queue_file):
    """ Open the supplied json queue file and return a list of queued job ids """
    jobs = {}
    job_ids = []
    with open(queue_file, 'r') as f:
        jobs = json.load(f)

    for job, meta in jobs.iteritems():
        if 'job_id' in meta and meta['status_bucket']['status'] == 'QUEUED':
            job_ids.append(meta['job_id'])

    return job_ids

def saveStatus(queue_file, job_ids):
    """
    Allow this script to update the test's statuses to 'waiting' to speed up
    the QueueManager (so the QueueManager doesn't need to ask qstat again, if
    a test is 'waiting'). Only do this for tests that are 'queued'.
    """
    jobs = {}
    with open(queue_file, 'r') as f:
        jobs = json.load(f)

    for job, meta in jobs.iteritems():
        if 'job_id' in meta and meta['job_id'] in job_ids and meta['status_bucket']['status'] == 'QUEUED':
            meta['status_bucket']['status'] = 'WAITING'

    if jobs:
        with open(queue_file, 'w') as f:
            json.dump(jobs, f, indent=2)

def usage():
    print('Supply a path to json queue file. Multiple files are supported, in which case all'
          'tests in all json files must be finished for this script to exit with 0.')
    sys.exit(1)

if __name__ == '__main__':
    args = sys.argv[1:]
    if len(args) == 0:
        usage()
    for queue_file in args:
        if os.path.exists(queue_file):
            job_ids = getJobIDs(queue_file)
            if not isFinished(queue_file, job_ids):
                sys.exit(1)
        else:
            print('Queue file does not exist: %s' %(queue_file))
            sys.exit(1)
