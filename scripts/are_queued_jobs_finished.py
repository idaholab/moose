#!/usr/bin/env python3
"""
Check if all jobs provided in PBS job file has finished.
  exit(0) if all jobs are finished, exit(1) if not.
"""
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import os
import json
import time
import argparse
import subprocess
from datetime import datetime, timedelta

def load_json(job_file):
    """ return PBS job dictionary """
    if os.path.exists(job_file):
        with open(job_file, 'r', encoding="utf-8") as pbs_job:
            try:
                return json.load(pbs_job)
            except json.decoder.JSONDecodeError:
                sys.exit('Not a valid json file')
    sys.exit(f'File not found: {job_file}')

def qstat_results(job_id):
    """ Run qstat and return (stdout, stderr) """
    with subprocess.Popen(['qstat', '-xfF', 'json', job_id],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE) as qstat_process:
        return qstat_process.communicate()

def yield_jobs(job_data):
    """ return key value dictionary pairs """
    for path_key, meta_data in job_data.items():
        yield path_key, meta_data

def not_finished(job_array):
    """
    Return True at the first opportunity when we find a reason that not every
    job has completed. Check for the quick stuff first, then do the expensive
    `qstat` command.
    """
    for job_data in job_array:
        for path, meta in yield_jobs(job_data):
            # Not a Job artifact (TestHarness argument/option, like -i, the scheduler, etc)
            if not isinstance(meta, dict):
                continue

            # Results file exist (job therefore finished)
            if os.path.exists(os.path.join(path, '.previous_test_results.json')):
                continue

            # Job was skipped, or otherwise not launched via PBS
            if not meta.get('RunPBS', False):
                continue

            # Ask qstat
            if 'RunPBS' in meta:
                job_id = meta['RunPBS']['ID'].replace('\n', '')
                qstat_command_result = qstat_results(job_id)

                # Error running qstat (job finished in any case)
                if qstat_command_result[1]:
                    continue

                # Catch json parsing errors (job finished in any case)
                try:
                    json_out = json.loads(qstat_command_result[0])
                # JobID no longer exists (stale after 1 week)
                except json.decoder.JSONDecodeError:
                    continue

                # If Exit_status exists, this job completed
                if json_out['Jobs'][job_id].get('Exit_status', False):
                    continue

            # If we made it this far, the job is still queued, running, etc
            return True

def parse_args(argv):
    """ parses arguments """
    if len(argv) == 0:
        print_usage()
    parser = argparse.ArgumentParser(description='Check if PBS has completed all jobs')
    parser.add_argument('-w', '--wait', nargs='?', metavar='int', type=int,
                        help='the max time to wait in seconds before giving up (a setting of'
                        ' -1 means forever)')
    return parser.parse_known_args(argv)

def print_usage():
    """ Print usage and then exit(1) """
    print('Supply a path to json queue file. Multiple files are supported,\n'
          'in which case all tests in all json files must be finished for\n'
          'this script to exit with 0.')
    sys.exit(1)

def main(args):
    """ check if all jobs in all supplied PBS files have finished """
    options, job_list = args
    job_array = []
    for queue_file in job_list:
        job_array.append(load_json(queue_file))
    if options.wait == -1:
        try:
            while not_finished(job_array):
                time.sleep(5)
        except KeyboardInterrupt:
            return 'Keyboard Interrupt'
    elif options.wait:
        time_start = datetime.now()
        try:
            while (datetime.now() - time_start) < timedelta(seconds=options.wait):
                if not_finished(job_array):
                    time.sleep(5)
                else:
                    return 0
        except KeyboardInterrupt:
            return 'Keyboard Interrrupt'
        return 'Allotted time exceeded'
    else:
        if not_finished(job_array):
            return 'Jobs still running'
    return 0

if __name__ == '__main__':
    sys.exit(main(parse_args(sys.argv[1:])))
