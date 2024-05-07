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
from TestHarness.runners.PBSRunner import PBSRunner
from timeit import default_timer as clock
from PBScodes import *
import paramiko
import jinja2

## This Class is responsible for maintaining an interface to the PBS scheduling syntax
class RunPBS(RunParallel):
    @staticmethod
    def validParams():
        params = RunParallel.validParams()
        params.addParam('queue_template', os.path.join(os.path.abspath(os.path.dirname(__file__)), 'pbs_template'), "Location of the PBS template")
        return params

    class PBSJob:
        """
        Structure that represents the cached information about a PBS job
        """
        def __init__(self, id, command):
            # The PBS job identifier
            self.id = id
            # Whether or not this job is done; here done doesn't mean if it
            # was successful or not, just if it is not running/queued anymore
            self.done = False
            # The exit code of the command that was ran (if any)
            self.exit_code = None
            # The job state as defined by PBS
            self.state = None
            # The command that was ran within the qsub script
            self.command = command
            # Whether or not this job was killed; used so what we don't
            # bother killing a job multiple times
            self.killed = False

    def __init__(self, harness, params):
        RunParallel.__init__(self, harness, params)
        self.params = params
        self.options = harness.getOptions()

        # We don't want to report long running jobs here because we will
        # manually set jobs as RUNNING as we notice their PBS status change
        self.report_long_jobs = False
        # We don't want to enforce the timeout here because we don't want to
        # check it while the jobs are queued and PBS itself will handle the
        # timeout because the job itself will be forcefully killed by PBS
        self.enforce_timeout = False

        # Lock for accessing self.pbs_jobs
        self.pbs_jobs_lock = threading.Lock()
        # The last time statues were updated in getPBSJob() (if any)
        self.pbs_jobs_status_timer = None
        # How often to poll for status updates in getPBSJob()
        self.pbs_jobs_update_interval = 10
        # Map of Job -> PBSJob
        self.pbs_jobs = {}

        # The jump hostname for running PBS commands, if any
        self.pbs_ssh_host = self.options.queue_host
        # Setup the remote PBS host, if any (needed when submitted in a container)
        self.pbs_ssh = None
        # The lock for calling PBS commands via SSH, if any
        self.pbs_ssh_lock = None
        # Setup the jump host if provided
        if self.pbs_ssh_host:
            self.pbs_ssh_lock = threading.Lock()
            self.pbs_ssh = paramiko.SSHClient()
            self.pbs_ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            self.pbs_ssh.connect(self.pbs_ssh_host)

        # Load the PBS template
        template_path = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'pbs_template')
        self.default_template = open(template_path, 'r').read()

        if os.environ.get('APPTAINER_CONTAINER'):
            if not self.pbs_ssh_host:
                print('ERROR: --pbs-host must be set when using --pbs within apptainer')
                sys.exit(1)
            if not self.options.queue_source_command:
                default_pre_source =  os.path.join(os.path.abspath(os.path.dirname(__file__)), 'pbs_source_apptainer')
                self.options.queue_source_command = default_pre_source
                print(f'INFO: Setting --pbs-pre-source={default_pre_source}')
        if self.options.queue_source_command and not os.path.exists(self.options.queue_source_command):
            print(f'ERROR: --pbs-pre-source path {self.options.queue_source_command} does not exist')
            sys.exit(1)

    class CallPBSException(Exception):
        """Exception class for providing extra context for PBS submission errors"""
        def __init__(self, run_pbs, description, command, result=None):
            message = f'{description}'
            if run_pbs.pbs_ssh:
                message += f' on host "{run_pbs.pbs_ssh_host}"'
            message += f'\nCommand: {command}'
            if result:
                message += f'\n\nResult:\n{result}'
            super().__init__(message)

    def callPBS(self, command):
        """Wrapper for calling a PBS command (qsub, qstat, etc) that supports
        SSH-ing to another host as needed when calling from within apptainer"""
        if not self.pbs_ssh:
            raise Exception('PBS not currently supported outside of a container')

        with self.pbs_ssh_lock:
            try:
                _, stdout, stderr = self.pbs_ssh.exec_command(command)
                exit_code = stdout.channel.recv_exit_status()
                result = ''.join(stdout.readlines())
                if exit_code != 0:
                    result += ''.join(stderr.readlines())
            except Exception as e:
                raise RunPBS.CallPBSException(self, 'Failed to execute remote PBS command', command) from e
            return exit_code, result.rstrip()

    def availableSlots(self, params):
        return 250, False

    def getPBSJobName(self, job):
        """Gets the name of the PBS job given a tester

        PBS doesn't like ":" or "/", hence changing them to "."
        """
        return job.getTestName().replace(':', '.').replace('/', '.')

    def getPBSJobOutputPathPrefix(self, job):
        """Gets the absolute path prefix for a PBS job"""
        return os.path.join(job.getTestDir(), "pbs_" + job.getTestNameShort().replace('/', '.'))

    def getPBSJobOutputPath(self, job):
        """Gets the absolute path for stdout/stderr for a PBS job"""
        return self.getPBSJobOutputPathPrefix(job) + '.out'

    def getPBSJobSubmissionPath(self, job):
        """Gets the aboslute path for the qsub script for a PBS job"""
        return self.getPBSJobOutputPathPrefix(job) + '.qsub'

    @staticmethod
    def parseMPICommand(command):
        find_mpi = re.search('^(mpiexec -n [0-9]+ )', command)
        if find_mpi is not None:
            return find_mpi.group(1)
        return None

    def submitJob(self, job):
        """Submits a PBS job"""
        tester = job.getTester()
        options = self.options

        # The qsub script we're going to write to
        qsub_file = self.getPBSJobSubmissionPath(job)
        # The combined stdout+stderr from the PBS job
        output_file = self.getPBSJobOutputPath(job)
        # Clean these two files
        for file in [qsub_file, output_file]:
            if os.path.exists(file):
                os.remove(file)

        # Set up the command. We have special logic here for when we're using apptainer,
        # where we need to put the MPI command outside of the apptainer call
        full_command = ''
        command = tester.getCommand(options)
        mpi_command = self.parseMPICommand(command)
        if mpi_command:
            command = command.replace(mpi_command, '')
            full_command += mpi_command

        APPTAINER_CONTAINER = os.environ.get('APPTAINER_CONTAINER')
        if APPTAINER_CONTAINER:
            apptainer_cmd = f'apptainer exec {APPTAINER_CONTAINER}'
            full_command += f'{apptainer_cmd} '
        # The double quotes around the exec command here are important as apptainer exec
        # doesn't work well with our command line arguments for some reason
        full_command += f'"{command}"'

        num_procs = tester.getProcs(options)
        num_threads = tester.getThreads(options)
        walltime = str(datetime.timedelta(seconds=tester.getMaxTime()))

        # Set up the template
        template_env = {'NAME': self.getPBSJobName(job),
                        'SELECT': f'{num_procs}:mpiprocs=1:ncpus={num_threads}',
                        'WALLTIME': walltime,
                        'PROJECT': self.options.queue_project,
                        'OUTPUT': output_file,
                        'PLACE': 'scatter',
                        'SUBMITTED_HOSTNAME': socket.gethostname(),
                        'CWD': tester.getTestDir(),
                        'COMMAND': full_command,
                        'ENDING_COMMENT': self.getOutputEndingComment()}
        if self.options.queue_queue:
            template_env['QUEUE'] = self.options.queue_queue
        if self.options.queue_source_command:
            template_env['SOURCE_COMMAND'] = self.options.queue_source_command

        # Build the script
        jinja_env = jinja2.Environment()
        definition_template = jinja_env.from_string(self.default_template)
        jinja_env.trim_blocks = True
        jinja_env.lstrip_blocks = True
        script = definition_template.render(**template_env)

        # Write the script
        open(qsub_file, 'w').write(script)

        # qsub submission command
        qsub_command = [f'cd {tester.getTestDir()}']
        qsub_command += [f'qsub {qsub_file}']
        qsub_command = '; '.join(qsub_command)

        # Set what we've ran for this job so that we can
        # potentially get the context in an error
        command_ran = qsub_command
        if self.pbs_ssh:
            command_ran = f"ssh {self.pbs_ssh_host} '{qsub_command}'"
        job.getTester().setCommandRan(command_ran)

        # Do the submission; this is thread safe
        # Eventually we might want to make this a pool so we can submit multiple
        # jobs at the same time
        exit_code, result = self.callPBS(qsub_command)

        # Nonzero return code
        if exit_code != 0:
            raise self.CallPBSException(self, 'qsub failed', qsub_command, result)

        # Make sure the job ID is something we'd expect
        job_id = result
        search = re.search('^[0-9]+.[a-zA-Z0-9_-]+$', job_id)
        if not search:
            raise self.CallPBSException(self, f'qsub has unexpected ID "{job_id}"', qsub_command)

        # Job has been submitted, so set it as queued
        job.addCaveats(job_id)
        self.setAndOutputJobStatus(job, job.queued)

        # Setup the job in the status map
        with self.pbs_jobs_lock:
            if job in self.pbs_jobs:
                raise Exception('Job has already been submitted')
            self.pbs_jobs[job] = self.PBSJob(job_id, full_command)

    def killJob(self, job):
        """Kills a PBS job"""
        with self.pbs_jobs_lock:
            if job not in self.pbs_jobs:
                return
            pbs_job = self.pbs_jobs[job]
            if pbs_job.done or pbs_job.killed:
                return
            job_id = self.pbs_jobs[job].id

        # Don't care about whether or not this failed
        self.callPBS(f'qdel {job_id}')

    def killRemaining(self, keyboard=False):
        """Kills all currently running PBS jobs"""
        job_ids = []
        with self.pbs_jobs_lock:
            for pbs_job in self.pbs_jobs.values():
                if not pbs_job.done:
                    job_ids.append(pbs_job.id)

        # Don't care about whether or not this failed
        self.callPBS(f'qdel {" ".join(job_ids)}')

        with self.pbs_jobs_lock:
            for pbs_job in self.pbs_jobs.values():
                if not pbs_job.done:
                    pbs_job.killed = True

        RunParallel.killRemaining(self, keyboard)

    def buildRunner(self, job, options):
        return PBSRunner(job, options, self)

    def getOutputEndingComment(self):
        """Gets the text we append to the PBS stderr+stdout file to desginate
        that it is complete"""
        return 'Completed TestHarness RunPBS job'

    def getPBSJob(self, job):
        """Gets the PBSJob object for a given Job

        This will periodically update the PBSJob in a thread safe manner so
        that we are not constantly calling qstat for every call."""

        with self.pbs_jobs_lock:
            # If this is the first time seeing this job, initialize it in the list
            if job not in self.pbs_jobs:
                raise Exception('Failed to get status for unsubmitted job')

            # Only update the statues periodically as this is called across threads
            if self.pbs_jobs_status_timer is None or ((clock() - self.pbs_jobs_status_timer) > self.pbs_jobs_update_interval):
                # Obtain the IDs of jobs that are active that we need to poll for
                active_job_ids = []
                for job, pbs_job in self.pbs_jobs.items():
                    if not pbs_job.done:
                        active_job_ids.append(pbs_job.id)

                # Poll for all of the jobs within a single call
                cmd = ['qstat', '-xf', '-F', 'json'] + active_job_ids
                exit_code, result = self.callPBS(' '.join(cmd))
                if exit_code != 0:
                    raise self.CallPBSException(self, 'Failed to get job status', cmd, result)

                # Register that we've updated the status
                self.pbs_jobs_status_timer = clock()

                # Attempt to parse the status from the jobs
                try:
                    json_result = json.loads(result)
                    job_results = json_result['Jobs']

                    for job, pbs_job in self.pbs_jobs.items():
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
                    raise self.CallPBSException(self, f'Failed to parse collective job status', cmd, result) from e

            return self.pbs_jobs[job]
