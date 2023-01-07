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


def isNotFinished(jobs):
    for path, meta in jobs.yieldJobsResultPath():
        if os.path.exists(path):
            job_id = meta['RunPBS']['ID'].split('.')[0]
            qstat_process = subprocess.Popen([ 'qstat' , '-xf', job_id], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            qstat_result = qstat_process.communicate()[0].decode('utf-8')
            job_status = re.findall(r'job_state = (.*)', qstat_result)
            if (job_status != ['F'] and job_status != ['X'] and job_status != ['U'] and job_status  != ['S'] and job_status != []):
                return True



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
            sys.exit(1)
