#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys, re, json, socket, datetime, threading
from RunParallel import RunParallel
from RunHPC import RunHPC
from timeit import default_timer as clock
from PBScodes import *
import jinja2

## This Class is responsible for maintaining an interface to the PBS scheduling syntax
class RunPBS(RunHPC):
    """
    Scheduler for HPC jobs that run with PBS.
    """
    @staticmethod
    def validParams():
        params = RunParallel.validParams()
        params.addParam('queue_template', os.path.join(os.path.abspath(os.path.dirname(__file__)), 'pbs_template'), "Location of the PBS template")
        return params

    def __init__(self, harness, params):
        super().__init__(harness, params)

        # Load the PBS template
        template_path = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'pbs_template')
        self.default_template = open(template_path, 'r').read()

    def _submitJob(self, job, job_data):
        tester = job.getTester()
        options = self.options

        # Add MOOSE's python path for python scripts
        moose_python = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)), '../..'))

        # Set up the template
        template_env = {'NAME': job_data.name,
                        'SELECT': f'{job_data.num_procs}:mpiprocs=1:ncpus={job_data.num_threads}',
                        'WALLTIME': job_data.walltime,
                        'PROJECT': self.options.queue_project,
                        'OUTPUT': job_data.output_file,
                        'PLACE': 'scatter',
                        'TEST_NAME': tester.getTestName(),
                        'SUBMITTED_HOSTNAME': socket.gethostname(),
                        'CWD': tester.getTestDir(),
                        'COMMAND': job_data.command,
                        'ESCAPED_COMMAND': json.dumps(job_data.command),
                        'ENDING_COMMENT': self.getOutputEndingComment(),
                        'MOOSE_PYTHONPATH': moose_python,
                        'OUTPUT_FILES': job_data.output_files}
        if self.options.queue_queue:
            template_env['QUEUE'] = options.queue_queue
        if self.options.queue_source_command:
            template_env['SOURCE_FILE'] = options.queue_source_command
        if self.source_contents:
            template_env['SOURCE_CONTENTS'] = self.source_contents

        # Build the script
        jinja_env = jinja2.Environment()
        definition_template = jinja_env.from_string(self.default_template)
        jinja_env.trim_blocks = True
        jinja_env.lstrip_blocks = True
        script = definition_template.render(**template_env)

        # Write the script
        open(job_data.submission_file, 'w').write(script)

        # qsub submission command
        qsub_command = [f'cd {tester.getTestDir()}']
        qsub_command += [f'qsub {job_data.submission_file}']
        qsub_command = '; '.join(qsub_command)

        # Set what we've ran for this job so that we can
        # potentially get the context in an error
        command_ran = qsub_command
        if self.ssh:
            command_ran = f"ssh {self.ssh_host} '{qsub_command}'"
        job.getTester().setCommandRan(command_ran)

        # Do the submission; this is thread safe
        # Eventually we might want to make this a pool so we can submit multiple
        # jobs at the same time
        exit_code, result = self.callHPC(qsub_command)

        # Nonzero return code
        if exit_code != 0:
            raise self.CallHPCException(self, 'qsub failed', qsub_command, result)

        # Make sure the job ID is something we'd expect
        job_id = result
        search = re.search('^[0-9]+.[a-zA-Z0-9_-]+$', job_id)
        if not search:
            raise self.CallHPCException(self, f'qsub has unexpected ID "{job_id}"', qsub_command)

        return job_id

    def updateJobs(self):
        # Obtain the IDs of jobs that are active that we need to poll for
        active_job_ids = []
        for job, pbs_job in self.hpc_jobs.items():
            if not pbs_job.done:
                active_job_ids.append(pbs_job.id)

        # Poll for all of the jobs within a single call
        cmd = ['qstat', '-xf', '-F', 'json'] + active_job_ids
        exit_code, result = self.callHPC(' '.join(cmd))
        if exit_code != 0:
            raise self.CallHPCException(self, 'Failed to get job status', cmd, result)

        # Attempt to parse the status from the jobs
        try:
            json_result = json.loads(result)
            job_results = json_result['Jobs']

            for job, pbs_job in self.hpc_jobs.items():
                # We're only updating jobs that aren't done yet
                if pbs_job.done:
                    continue

                # This job's result from the qstat command
                job_result = job_results[pbs_job.id]
                exit_code = job_result.get('Exit_status')
                if exit_code is not None:
                    exit_code = int(exit_code)
                state = job_result.get('job_state')
                substate = job_result.get('substate')
                terminated = int(substate) == 91 if substate else False
                done = exit_code is not None or terminated

                # Get the job state, and report running if it switched to running
                if state == 'R' and pbs_job.state != 'R':
                    self.setAndOutputJobStatus(job, job.running)

                # Update the PBSJob structure
                pbs_job.done = done
                pbs_job.exit_code = exit_code
                pbs_job.state = state

                # Mark the job as terminated (past walltime, over resources, killed)
                if terminated:
                    job.setStatus(job.error, 'PBS JOB TERMINATED')
        except Exception as e:
            raise self.CallHPCException(self, f'Failed to parse collective job status', cmd, result) from e

    def killJob(self, job):
        """Kills a PBS job"""
        with self.hpc_jobs_lock:
            if job not in self.hpc_jobs:
                return
            hpc_job = self.hpc_jobs[job]
            if hpc_job.done or hpc_job.killed:
                return
            job_id = self.hpc_jobs[job].id

        # Don't care about whether or not this failed
        self.callHPC(f'qdel {job_id}')

    def killRemaining(self, keyboard=False):
        """Kills all currently running PBS jobs"""
        job_ids = []
        with self.hpc_jobs_lock:
            for hpc_job in self.hpc_jobs.values():
                if not hpc_job.done:
                    job_ids.append(hpc_job.id)

        # Don't care about whether or not this failed
        self.callHPC(f'qdel {" ".join(job_ids)}')

        with self.hpc_jobs_lock:
            for hpc_job in self.hpc_jobs.values():
                if not hpc_job.done:
                    hpc_job.killed = True

        RunParallel.killRemaining(self, keyboard)
