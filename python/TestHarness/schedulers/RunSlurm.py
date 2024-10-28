#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
from datetime import datetime
from RunHPC import RunHPC

## This Class is responsible for maintaining an interface to the slurm scheduling syntax
class RunSlurm(RunHPC):
    """
    Scheduler class for the slurm HPC scheduler.
    """
    def __init__(self, harness, params):
        super().__init__(harness, params)

        # Slurm is significantly better at job status, so we can
        # update all at the same time
        self.update_hpc_jobs_chunk_size = 1000

    def updateHPCJobs(self, hpc_jobs):
        # Poll for all of the jobs within a single call
        active_job_ids = ','.join([x.id for x in hpc_jobs])
        cmd = ['sacct', '-j', active_job_ids, '--parsable2', '--noheader',
               '-o', 'jobid,exitcode,state,reason,start,end']
        exit_code, result, _ = self.callHPC(self.CallHPCPoolType.status, ' '.join(cmd))
        if exit_code != 0:
            return False

        # Parse the status from the jobs
        statuses = {}
        for status in result.splitlines():
            # jobid,exitcode,state,reason are split by |
            status_split = status.split('|')
            # Slurm has sub jobs under each job, and we only care about the top-level job
            id = status_split[0]
            if not id.isdigit():
                continue
            # exitcode is <val>:<val>, where the first value is the
            # exit code of the process, the second is a slurm internal code
            statuses[id] = {'exitcode': int(status_split[1].split(':')[0]),
                            'state': status_split[2],
                            'reason': status_split[3],
                            'start': status_split[4],
                            'end': status_split[5]}

        # Update the jobs that we can
        for hpc_job in hpc_jobs:
            # Helper for parsing a time
            def parse_time(time):
                if time:
                    return datetime.strptime(time, '%Y-%m-%dT%H:%M:%S').timestamp()
                return None

            # Slurm jobs are sometimes not immediately available
            status = statuses.get(hpc_job.id)
            if status is None:
                continue

            # The slurm job state; see slurm.schedmd.com/squeue.html#lbAG
            state = status['state']

            with hpc_job.getLock():
                # Job wasn't running and it's no longer pending, so it
                # is running or has at least ran
                if state != 'PENDING' and hpc_job.state != hpc_job.State.running:
                    start_time = parse_time(status['start'])
                    self.setHPCJobRunning(hpc_job, start_time)

                # Job was running and isn't running anymore, so it's done
                if hpc_job.state == hpc_job.State.running and state not in ['RUNNING', 'COMPLETING']:
                    exit_code = int(status['exitcode'])
                    if state == 'FAILED' and exit_code == 0:
                        raise Exception(f'Job {hpc_job.id} has unexpected exit code {exit_code} with FAILED state')

                    job = hpc_job.job

                    # Job has timed out; setting a timeout status means that this
                    # state is recoverable
                    if state == 'TIMEOUT':
                        job.setStatus(job.timeout, 'SLURM JOB TIMEOUT')
                    # If a job COMPLETED, it's done with exit code 0 so everything
                    # went well. If it FAILED, it finished but returned with a
                    # non-zero exit code, which will be handled by the Tester.
                    elif state not in ['FAILED', 'COMPLETED']:
                        self.setHPCJobError(hpc_job, f'SLURM ERROR: {state}', f'has state "{state}"')

                    end_time = parse_time(status['end'])
                    self.setHPCJobDone(hpc_job, exit_code, end_time)

        # Success
        return True

    def getHPCSchedulerName(self):
        return 'slurm'

    def getHPCSubmissionCommand(self):
        return 'sbatch'

    def getHPCQueueCommand(self):
        return 'scontrol release'

    def getHPCCancelCommand(self):
        return 'scancel'

    def getHPCJobIDVariable(self):
        return 'SLURM_JOB_ID'

    def parseHPCSubmissionJobID(self, result):
        search = re.search('^Submitted batch job ([0-9]+)$', result)
        if not search:
            raise Exception(f'Failed to parse job ID from "{result}"')
        return str(search.group(1))

