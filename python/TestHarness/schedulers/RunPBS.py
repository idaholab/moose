#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, json
import datetime
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

    def updateHPCJobs(self, hpc_jobs):
        # Poll for all of the jobs within a single call
        cmd = ['qstat', '-xf', '-F', 'json'] + [x.id for x in hpc_jobs]
        exit_code, result, _ = self.callHPC(self.CallHPCPoolType.status, ' '.join(cmd))
        if exit_code != 0:
            return False

        # Parse the status from the jobs
        json_result = json.loads(result)
        job_results = json_result['Jobs']

        for hpc_job in hpc_jobs:
            # This job's result from the qstat command
            job_result = job_results[hpc_job.id]
            exit_code = job_result.get('Exit_status')
            if exit_code is not None:
                exit_code = int(exit_code)
            state = job_result.get('job_state')
            obittime = job_result.get('obittime')

            with hpc_job.getLock():
                job = hpc_job.job

                # Helper for parsing timings
                def parse_time(name):
                    time_format = '%a %b %d %H:%M:%S %Y'
                    entry = job_result.get(name)
                    if not entry:
                        return None

                    try:
                        return datetime.datetime.strptime(entry, '%a %b %d %H:%M:%S %Y').timestamp()
                    except:
                        self.setHPCJobError(hpc_job, 'FAILED TO PARSE TIMING',
                                            f'Failed to parse time "{time}" from entry "{name}"')
                        return None

                # Job is queued and it has switched to running
                if hpc_job.state == hpc_job.State.queued:
                    start_time = parse_time('stime')
                    if start_time:
                        self.setHPCJobRunning(hpc_job, start_time)

                # The job is held, so we're going to consider it a failure and
                # will also try to cancel it so that it doesn't hang around
                if state == 'H' and (job_result.get('Hold_Types') != 'u' or self.options.hpc_no_hold):
                    self.setHPCJobError(hpc_job, 'PBS JOB HELD', 'was held; killed job')
                    exit_code = 1
                    try:
                        self.killHPCJob(hpc_job, lock=False) # no lock; we're already in one
                    except:
                        pass

                # Job finished before it started, so something killed it
                if state == 'F' and exit_code is None:
                    self.setHPCJobError(hpc_job, 'PBS JOB KILLED', 'was killed')
                    exit_code = 1

                # If we have a finished time or an error, we're done
                if exit_code is not None:
                    if exit_code < 0:
                        name, reason = PBS_User_EXITCODES.get(exit_code, ('TERMINATED', 'Unknown reason'))
                        # Job failed to start outside of our submission script, so
                        # try it again a few times. This was implemented due to a
                        # common issue on lemhi
                        if name == 'JOB_EXEC_FAIL2' and hpc_job.num_resubmit <= 5:
                            self.resubmitHPCJob(hpc_job)
                            continue
                        # Job timed out; give this a special timeout status because
                        # it is then marked as recoverable (could try running again)
                        if name == 'JOB_EXEC_KILL_WALLTIME':
                            job.setStatus(job.timeout, 'PBS JOB TIMEOUT')
                        # Special status where the job failed to start due to a PBS
                        # issue and will be started again, so there's nothing to do
                        elif name in ['JOB_EXEC_HOOK_RERUN', 'JOB_EXEC_RETRY']:
                            self.setHPCJobQueued(hpc_job)
                            continue
                        # Everything else should be an error
                        else:
                            self.setHPCJobError(hpc_job, f'PBS ERROR: {name}', f'was terminated with reason: {reason}')
                    # Job was killed with a signal
                    elif exit_code >= 128:
                        self.setHPCJobError(hpc_job, 'PBS JOB KILLED', 'was killed by a signal')

                    # Parse end time if possible. PBS is all over the place on this one. Sometimes
                    # walltime is available, sometimes it isn't. We also have obittime, but that
                    # time seems to be longer than the actual run.
                    end_time = None
                    # First try to get it from the walltime (sometimes this is 0...). We'll fake
                    # this a bit and just add the walltime to the start time
                    stime = parse_time('stime')
                    resources_used = job_result.get('resources_used')
                    if stime and resources_used:
                        walltime = resources_used.get('walltime')
                        if walltime:
                            search = re.search(r'^(\d+):(\d{2}):(\d{2})$', walltime)
                            if search:
                                walltime_sec = datetime.timedelta(hours=int(search.group(1)),
                                                                  minutes=int(search.group(2)),
                                                                  seconds=int(search.group(3))).total_seconds()
                                if walltime_sec != 0:
                                    end_time = stime + walltime_sec
                            else:
                                self.setHPCJobError(hpc_job, 'WALLTIME PARSE ERROR',
                                                    f'Failed to parse walltime from "{walltime}"')
                    # If we don't have it yet, use the obit time
                    if not end_time:
                        obittime = parse_time('obittime')
                        if obittime:
                            end_time = obittime

                    self.setHPCJobDone(hpc_job, exit_code, end_time)

        # Success
        return True

    def getHPCSchedulerName(self):
        return 'pbs'

    def getHPCSubmissionCommand(self):
        return 'qsub'

    def getHPCQueueCommand(self):
        return 'qrls'

    def getHPCCancelCommand(self):
        return 'qdel -W force'

    def getHPCJobIDVariable(self):
        return 'PBS_JOBID'

    def parseHPCSubmissionJobID(self, result):
        search = re.search('^[0-9]+.[a-zA-Z0-9_-]+$', result)
        if not search:
            raise Exception(f'qsub has unexpected ID {result}')
        return result

    def callHPCShouldRetry(self, pool_type, result: str):
        # If we're submitting/releasing/getting a status and cannot connect
        # to the scheduler, we can retry
        if pool_type in [self.CallHPCPoolType.submit,
                         self.CallHPCPoolType.queue,
                         self.CallHPCPoolType.status]:
            return 'pbs_iff: cannot connect to host' in result
        return False
