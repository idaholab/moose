#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, json
from RunHPC import RunHPC
from PBScodes import PBS_User_EXITCODES
from TestHarness import util

## This Class is responsible for maintaining an interface to the PBS scheduling syntax
class RunPBS(RunHPC):
    """
    Scheduler class for the PBS HPC scheduler.
    """
    def augmentJobSubmission(self, submission_env):
        if self.options.hpc_queue:
            submission_env['QUEUE'] = self.options.hpc_queue

    def updateHPCJobs(self, active_hpc_jobs):
        # Poll for all of the jobs within a single call
        cmd = ['qstat', '-xf', '-F', 'json'] + [x.id for x in active_hpc_jobs]
        exit_code, result, _ = self.callHPC(' '.join(cmd))
        if exit_code != 0:
            return False

        # Parse the status from the jobs
        json_result = json.loads(result)
        job_results = json_result['Jobs']

        for hpc_job in active_hpc_jobs:
            job = hpc_job.job

            # This job's result from the qstat command
            job_result = job_results[hpc_job.id]
            exit_code = job_result.get('Exit_status')
            if exit_code is not None:
                exit_code = int(exit_code)
            state = job_result.get('job_state')

            # The job has switched to running
            if state == 'R' and not hpc_job.getRunning():
                self.setHPCJobRunning(hpc_job)

            # The job is held, so we're going to consider it a failure and
            # will also try to cancel it so that it doesn't hang around
            if state == 'H':
                self.setHPCJobError(hpc_job, 'PBS JOB HELD', 'was held; killed job')
                exit_code = 1
                try:
                    self.killJob(job, lock=False) # no lock; we're already in one
                except:
                    pass

            # Job finished before it started, so something killed it
            if state == 'F' and exit_code is None:
                self.setHPCJobError(hpc_job, 'PBS JOB KILLED', 'was killed')
                exit_code = 1

            # If we were running but now we're done, we're not running anymore
            if exit_code is not None:
                if exit_code < 0:
                    name, reason = PBS_User_EXITCODES.get(exit_code, ('TERMINATED', 'Unknown reason'))
                    # Job timed out; give this a special timeout status because
                    # it is then marked as recoverable (could try running again)
                    if name == 'JOB_EXEC_KILL_WALLTIME':
                        job.setStatus(job.timeout, 'TIMEOUT')
                    # Special status where the job failed to start due to a PBS
                    # issue and will be started again, so there's nothing to do
                    elif name == 'JOB_EXEC_HOOK_RERUN':
                        self.setHPCJobQueued(hpc_job)
                        continue
                    # Everything else should be an error
                    else:
                        self.setHPCJobError(hpc_job, f'PBS ERROR: {name}', f'was terminated with reason: {reason}')
                # Job was killed with a signal
                elif exit_code >= 128:
                    job.setStatus(job.error, f'PBS JOB KILLED')

                self.setHPCJobDone(hpc_job, exit_code)

        # Success
        return True

    def getHPCSchedulerName(self):
        return 'pbs'

    def getHPCSubmissionCommand(self):
        return 'qsub'

    def getHPCCancelCommand(self):
        return 'qdel'

    def getHPCJobIDVariable(self):
        return 'PBS_JOBID'

    def parseHPCSubmissionJobID(self, result):
        search = re.search('^[0-9]+.[a-zA-Z0-9_-]+$', result)
        if not search:
            raise Exception(f'qsub has unexpected ID {result}')
        return result
