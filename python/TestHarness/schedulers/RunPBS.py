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
        submission_env['PLACE'] = self.options.hpc_place
        if self.options.hpc_queue:
            submission_env['QUEUE'] = self.options.hpc_queue

    def updateJobs(self, active_job_ids):
        # Poll for all of the jobs within a single call
        cmd = ['qstat', '-xf', '-F', 'json'] + active_job_ids
        exit_code, result, _ = self.callHPC(' '.join(cmd))
        if exit_code != 0:
            return False

        # Attempt to parse the status from the jobs
        try:
            json_result = json.loads(result)
            job_results = json_result['Jobs']

            for job, hpc_job in self.hpc_jobs.items():
                # We're only updating jobs that aren't done yet
                if hpc_job.done:
                    continue

                # This job's result from the qstat command
                job_result = job_results[hpc_job.id]
                exit_code = job_result.get('Exit_status')
                if exit_code is not None:
                    exit_code = int(exit_code)
                state = job_result.get('job_state')

                # Get the job state, and report running if it switched to running
                if state == 'R' and not hpc_job.running:
                    hpc_job.running = True
                    self.setAndOutputJobStatus(job, job.running, caveats=True)

                # If we were running but now we're done, we're not running anymore
                if exit_code is not None:
                    hpc_job.running = False
                    hpc_job.done = True
                    hpc_job.exit_code = exit_code

                    # Negative exit code, means PBS set a reason
                    if exit_code < 0:
                        name, reason = PBS_User_EXITCODES.get(exit_code, ('TERMINATED', 'Unknown reason'))
                        job.setStatus(job.error, f'PBS ERROR: {name}')
                        job.appendOutput(util.outputHeader(f'PBS terminated job with reason: {reason}'))
                    # Job was killed with a signal
                    elif exit_code >= 128:
                        job.setStatus(job.error, f'PBS JOB KILLED')
        except Exception as e:
            raise self.CallHPCException(self, f'Failed to parse collective job status', cmd, result) from e

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
