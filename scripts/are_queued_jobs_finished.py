#!/usr/bin/env python3
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
            with open(json_file, 'r') as f:
                self.__job_data = json.load(f)
        else:
            raise Exception('File does not exist: %s' % (json_file))

    def yieldJobsResultPath(self):
        for k, v in self.__job_data.items():
            yield k, v

def runCommand(cmd, cwd=None):
    # On Windows it is not allowed to close fds while redirecting output
    p = subprocess.Popen(cmd,
                         cwd=cwd,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT,
                         close_fds=True,
                         shell=True)

    output = p.communicate()[0].decode('utf-8')
    if (p.returncode != 0):
        output = 'ERROR: ' + output
    return output

def isNotFinished(jobs):
    """
    Return True when we find a reason that not every job has completed.
    Check for the quick stuff first, then do the expensive `qstat` command.
    """
    for path, meta in jobs.yieldJobsResultPath():

        # Not a Job artifact (TestHarness argument/option, like -i, the scheduler, etc)
        if type(meta) != type({}):
            continue

        # Results file exist (job therefore finished)
        elif os.path.exists(os.path.join(path, '.previous_test_results.json')):
            continue

        # Job was skipped, or otherwise not launched via PBS
        elif not meta.get('RunPBS', False):
            continue

        # Ask qstat
        elif 'RunPBS' in meta:
            job_id = meta['RunPBS']['ID'].replace('\n', '')
            qstat_command_result = runCommand(f'qstat -xf -F json {job_id}')

            # Catch json parsing errors
            try:
                json_out = json.loads(qstat_command_result)
                job_meta = json_out['Jobs'][job_id]

                # If Exit_status exists, then this job completed
                if job_meta.get('Exit_status', False):
                    continue
                else:
                    print(f'STILL RUNNING: {job_id}')
                    return True

            # JobID no longer exists (stale after 1 week)
            except json.decoder.JSONDecodeError:
                # Job has completed if we make it here
                print('problem decoding data (week old)')
                continue

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
        if isNotFinished(jobs):
            sys.exit('NOT READY')
    print('READY')
    sys.exit(0)
