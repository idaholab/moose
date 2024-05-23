#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
from RunHPC import RunHPC

## This Class is responsible for maintaining an interface to the slurm scheduling syntax
class RunSlurm(RunHPC):
    """
    Scheduler class for the slurm HPC scheduler.
    """
    def __init__(self, harness, params):
        super().__init__(harness, params)

        # Slurm is quite a bit faster at updating
        self.hpc_jobs_update_interval = 5

    def updateJobs(self, active_job_ids):
        # Poll for all of the jobs within a single call
        cmd = ['sacct', '-j', ','.join(active_job_ids), '--parsable2', '--noheader', '-o', 'jobid,exitcode,state,reason']
        exit_code, result, _ = self.callHPC(' '.join(cmd))
        if exit_code != 0:
            return False

        # Attempt to parse the status from the jobs
        try:
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
                                'reason': status_split[3]}

            # Update the jobs that we can
            for job, hpc_job in self.hpc_jobs.items():
                # We're only updating jobs that aren't done yet
                if hpc_job.done:
                    continue
                # Slurm jobs are sometimes not immediately available
                status = statuses.get(hpc_job.id)
                if status is None:
                    continue

                state = status['state']
                if state != 'PENDING' and not hpc_job.running:
                    hpc_job.running = True
                    self.setAndOutputJobStatus(job, job.running, caveats=True)
                if hpc_job.running and state not in ['RUNNING', 'COMPLETING']:
                    hpc_job.running = False
                    hpc_job.done = True
                    hpc_job.exit_code = status['exitcode']
                    if state not in ['FAILED', 'COMPLETED']:
                        job.setStatus(job.error, f'SLURM ERROR: {state}')
        except Exception as e:
            raise self.CallHPCException(self, f'Failed to parse collective job status', cmd, result) from e

        # Success
        return True

    def getHPCSchedulerName(self):
        return 'slurm'

    def getHPCSubmissionCommand(self):
        return 'sbatch'

    def getHPCCancelCommand(self):
        return 'scancel'

    def getHPCJobIDVariable(self):
        return 'SLURM_JOB_ID'

    def parseHPCSubmissionJobID(self, result):
        search = re.search('^Submitted batch job ([0-9]+)$', result)
        if not search:
            raise Exception(f'Failed to parse job ID from "{result}"')
        return str(search.group(1))

