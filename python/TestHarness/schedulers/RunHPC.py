#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import urllib.parse
from RunParallel import RunParallel
import threading, os, re, sys, datetime, shlex, socket, threading, time, urllib, contextlib, copy
from enum import Enum
import statistics
from collections import namedtuple

from multiprocessing.pool import ThreadPool
from TestHarness import util

class HPCJob:
    # The valid job states for a HPC job
    State = Enum('State', ['waiting', 'held', 'queued', 'running', 'done', 'killed'])

    """
    Structure that represents the cached information about an HPC job
    """
    def __init__(self, job):
        # The underlying Job
        self.job = job
        # The ID of the HPC job
        self.id = None
        # The command that was ran within the job
        self.command = None
        # The state that this job is in
        self.state = self.State.waiting
        # The exit code of the command that was ran (if any)
        self.exit_code = None
        # The number of times the job has been resubmitted
        self.num_resubmit = 0
        # Lock for accessing this object
        self.lock = threading.Lock()

    def getLock(self):
        """
        Gets the lock for this object.
        """
        return self.lock

    def get(self, key):
        """
        Thread-safe getter for a key
        """
        with self.getLock():
            return getattr(self, key)

    def getState(self):
        """
        Thread-safe getter for the state
        """
        return self.get('state')

    def isKilled(self):
        """
        Thread-safe getter for whether or not this was killed
        """
        return self.getState() == self.State.killed

    def reset(self):
        """
        Resets the job state

        Not thread safe; should be called within a lock
        """
        self.id = None
        self.command = None
        self.state = self.State.waiting
        self.exit_code = None

class RunHPC(RunParallel):
    # The types for the pools for calling HPC commands
    CallHPCPoolType = Enum('CallHPCPoolType', ['submit', 'queue', 'status', 'kill'])

    """
    Base scheduler for jobs that are ran on HPC.
    """
    def __init__(self, harness, params):
        import paramiko

        super().__init__(harness, params)

        self.params = params
        self.options = harness.getOptions()

        # We don't want to report long running jobs here because we will
        # manually set jobs as RUNNING as we notice their HPC status change
        self.report_long_jobs = False
        # We don't want to enforce the timeout here because we don't want to
        # check it while the jobs are queued and HPC itself will handle the
        # timeout because the job itself will be forcefully killed by HPC
        self.enforce_timeout = False

        # Lock for accessing self.hpc_jobs
        self.hpc_jobs_lock = threading.Lock()
        # How often to poll (in sec) for status updates in getHPCJob()
        self.hpc_jobs_update_interval = 5
        # How many HPC jobs to update at a time in updateHPCJobs()
        # This needs to be an option because PBS is awful
        self.update_hpc_jobs_chunk_size = 50
        # Map of Job ID -> HPCJob
        self.hpc_jobs = {}
        # The thread that will update the HPCJobs
        self.hpc_jobs_updater = None

        # The pool of processes for running HPC scheduler commands
        # We have a pool so that we don't overwhelm the login node
        # with commands, and have a pool for each interaction type
        # so that those commands only compete with commands of the
        # other type
        self.call_hpc_pool = {}
        self.call_hpc_pool[self.CallHPCPoolType.submit] = ThreadPool(processes=5)
        if not self.options.hpc_no_hold: # only used with holding jobs
            self.call_hpc_pool[self.CallHPCPoolType.queue] = ThreadPool(processes=5)
        for val in [self.CallHPCPoolType.status, self.CallHPCPoolType.kill]:
            self.call_hpc_pool[val] = ThreadPool(processes=1)

        # The jump hostname for running commands, if any
        self.ssh_hosts = self.options.hpc_host
        # The SSH key to use for connections
        self.ssh_key_filenames = None
        # The threaded SSHClient objects, mapped by thread identifier
        # Tuple of (paramiko.SSHClient, str) where str is the hostname
        self.ssh_clients = None
        # The lock for calling commands via SSH,
        self.ssh_clients_lock = None
        # Setup the jump host if provided
        # We allow multitple hosts here to have backups
        if self.ssh_hosts:
            if isinstance(self.ssh_hosts, str):
                self.ssh_hosts = [self.ssh_hosts]

            self.ssh_clients = {}
            self.ssh_clients_lock = threading.Lock()

            # Try to find a key to use for each host. Paramiko doesn't
            # use any non-default keys by default, so we need to search
            # like this and apply them manually
            self.ssh_key_filenames = {}
            for host in self.ssh_hosts:
                try:
                    ssh_config = os.path.expanduser('~/.ssh/config')
                    config = paramiko.SSHConfig.from_path(ssh_config).lookup(host)
                    identityfile = config.get('identityfile')
                    if identityfile is not None and len(identityfile) > 0:
                        self.ssh_key_filenames[host] = identityfile[-1]
                except:
                    pass

        # Make sure that we can call commands up front, only if we're not re-running
        if not self.options.show_last_run:
            for val in self.CallHPCPoolType:
                if self.options.hpc_no_hold and val == self.CallHPCPoolType.queue:
                    continue
                self.callHPC(val, 'hostname')

        # Pool for submitJob(), so that we can submit jobs to be
        # held in the background without blocking
        self.submit_job_pool = None if self.options.hpc_no_hold else ThreadPool(processes=10)

        # Build the base submission environemnt for a job
        self.submit_env, self.app_exec_prefix, self.app_exec_suffix = self.setupRunEnvironment(harness)

        if os.environ.get('APPTAINER_CONTAINER'):
            if not self.ssh_hosts:
                print('ERROR: --hpc-host must be set when using HPC jobs within apptainer')
                sys.exit(1)
        else:
            if self.options.hpc_apptainer_bindpath:
                print('ERROR: --hpc-apptainer-bindpath is unused when not executing with apptainer')
                sys.exit(1)
            if self.options.hpc_apptainer_no_home:
                print('ERROR: --hpc-apptainer-no-home is unused when not executing with apptainer')
                sys.exit(1)

        if self.options.hpc_pre_source and not os.path.exists(self.options.hpc_pre_source):
            print(f'ERROR: --hpc-pre-source path {self.options.hpc_pre_source} does not exist')
            sys.exit(1)
        if self.options.hpc and self.options.pedantic_checks:
            print('ERROR: --hpc and --pedantic-checks cannot be used simultaneously')
            sys.exit(1)
        if self.options.hpc and self.options.jobs:
            print('ERROR: --hpc and -j|--jobs cannot be used simultaneously')
            sys.exit(1)

        # Load the pre-source if it exists
        self.pre_source_contents = None
        if self.options.hpc_pre_source:
            with open(self.options.hpc_pre_source, 'r') as f:
                self.pre_source_contents = f.read()

        # Load the submission template
        template_path = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'hpc_template')
        with open(template_path, 'r') as f:
            self.submission_template = f.read()

    class CallHPCException(Exception):
        """
        Exception class for providing extra context for HPC submission errors
        """
        def __init__(self, description, host, command, result=None):
            message = f'{description}'
            if host:
                message += f' on host "{host}"'
            message += f'\nCommand: {command}'
            if result:
                message += f'\n\nResult:\n{result}'
            super().__init__(message)

    def _getSSHClient(self, reconnect=False):
        """
        Gets a SSH client owned by a thread.

        This is threaded so that we can operate a few connections at once.
        """
        import paramiko

        process = threading.get_ident()
        with self.ssh_clients_lock:
            if process not in self.ssh_clients or reconnect:
                self.ssh_clients[process] = None
                for host in self.ssh_hosts:
                    client = paramiko.SSHClient()
                    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                    key_filename = self.ssh_key_filenames.get(host)
                    try:
                        client.connect(host, key_filename=key_filename)
                    except Exception as e:
                        print(f'WARNING: Failed to connect to HPC host {host}: {e}')
                        continue
                    self.ssh_clients[process] = (client, host)
                    break

            client_and_host = self.ssh_clients.get(process)
            if client_and_host is None:
                raise Exception(f'Failed to connect to SSH host(s) {", ".join(self.ssh_hosts)}')
            return client_and_host

    def _callSSH(self, command):
        """
        Calls a SSH command.

        Should only be used via apply with the self.call_hpc_pool.
        """
        client, host = self._getSSHClient()

        # Here we try twice, in the event that the connection was killed
        retry = False
        while True:
            try:
                client, host = self._getSSHClient(reconnect=retry)
                _, stdout, stderr = client.exec_command(command)
            except Exception as e:
                if not retry:
                    retry = True
                    continue
                raise RunHPC.CallHPCException('Failed to execute remote command', host, command) from e
            break

        exit_code = stdout.channel.recv_exit_status()
        result = ''.join(stdout.readlines())
        if exit_code != 0:
            result += ''.join(stderr.readlines())
        full_command = f"ssh {host} '{command}'"
        return exit_code, result.rstrip(), full_command

    def callHPC(self, pool_type, command: str, num_retries: int = 0, retry_time: float = 5):
        """
        Wrapper for calling a HPC command (qsub, qstat, etc) that supports
        SSH-ing to another host as needed when calling from within apptainer

        Set num_retires to retry the command this many times, waiting
        retry_time sec between each retry. The command will only be retried
        if self.callHPCShouldRetry() is True for that command. This lets
        us retry commands given known failures.

        Requires the "pool" to specify which command pool to use, of the
        RunHPC.CallHPCPoolType types.
        """
        if not self.ssh_hosts:
            raise Exception('HPC not currently supported outside of a container')

        exit_code = None
        result = None
        full_cmd = None

        for i in range(num_retries + 1):
            exit_code, result, full_cmd = self.call_hpc_pool[pool_type].apply(self._callSSH, (command,))
            if exit_code == 0:
                break
            if self.callHPCShouldRetry(pool_type, result):
                time.sleep(retry_time)
            else:
                break

        return exit_code, result, full_cmd

    def getJobSlots(self, job):
        # Jobs only use one slot because they are ran externally
        return 1

    def availableSlots(self, params):
        # Support managing 250 HPC jobs concurrently
        return 250, False

    @staticmethod
    def jobCaveat(hpc_job) -> str:
        """
        Gets the caveat associated with the job ID for a HPCJob
        """
        job_id = hpc_job.id
        assert job_id is not None
        return f'job={job_id}' if job_id.isdigit() else job_id

    def resubmitHPCJob(self, hpc_job):
        """
        Resumits the given HPCJob.

        The HPCJob must have already been submitted.

        This should be called from within the derived
        scheduler to resubmit.
        """
        assert hpc_job.state != hpc_job.State.waiting
        job = hpc_job.job
        job.removeCaveat(self.jobCaveat(hpc_job))
        hpc_job.job.addCaveats('resubmitted')
        hpc_job.reset()
        hpc_job.num_resubmit += 1
        self.submitJob(job, False, lock=False)

    def submitJob(self, job, hold, lock=True):
        """
        Method for submitting an HPC job for the given Job.

        The "hold" flag specifies whether or not to submit
        the job in a held state.

        Set lock=False if calling this within a method
        whether the HPC job lock is already obtained.

        Returns the resulting HPCJob.
        """
        import jinja2

        # If we're submitting this Job to be held, but the Job status isn't
        # currently held, it means that we've hit job in the submit_job_pool
        # that was submitted previously but has already been set to be skipped
        # (likely due to a prereq failure)
        # NOTE: This _is_ thread safe because StatusSystem is blocking
        if hold and not job.isHold():
            return None

        with self.hpc_jobs_lock:
            hpc_job = self.hpc_jobs.get(job.getID())

            # Job hasn't been recorded yet; set up with a waiting state
            if hpc_job is None:
                assert lock is True
                self.hpc_jobs[job.getID()] = HPCJob(job)
                hpc_job = self.hpc_jobs.get(job.getID())

        hpc_job_lock = hpc_job.getLock() if lock else contextlib.nullcontext()
        with hpc_job_lock:
            # Job has already been submitted
            if hpc_job.state != hpc_job.State.waiting:
                return hpc_job

            tester = job.getTester()
            options = self.options

            # If we have a separate output directory, we might need to create this
            # for the files that follow. This won't do anything if it exists and
            # it is thread safe
            job.createOutputDirectory()

            submission_script = self.getHPCJobSubmissionPath(job)
            output_file = self.getHPCJobOutputPath(job)
            result_file = self.getHPCJobResultPath(job)

            # Remove these files if they exist
            for file in [submission_script, output_file, result_file]:
                if os.path.exists(file):
                    os.remove(file)

            # Start building the jinja environment for the submission script
            submission_env = copy.deepcopy(self.submit_env)
            submission_env.update({'NAME': self.getHPCJobName(job),
                                   'CWD': tester.getTestDir(),
                                   'OUTPUT': output_file,
                                   'RESULT': result_file,
                                   'SUBMISSION_SCRIPT': submission_script,
                                   'WALLTIME': str(datetime.timedelta(seconds=tester.getMaxTime())),
                                   'TEST_SPEC': tester.getSpecFile(),
                                   'TEST_NAME': tester.getTestNameShort(),
                                   'NUM_PROCS': int(tester.getProcs(options)),
                                   'NUM_THREADS': int(tester.getThreads(options)),
                                   'PLACE': tester.getHPCPlace(options)})
            if hold:
                submission_env['HOLD'] = 1

            # Get the unescaped command
            command = tester.getCommand(options)

            # Parse out the mpi command from the command if we're wrapping
            # things around the mpi command
            mpi_prefix = ''
            if self.app_exec_prefix:
                mpi_command = self.parseMPICommand(command)
                if mpi_command:
                    mpi_prefix = mpi_command
                    command = command.replace(mpi_command, '')

            # Replace newlines, clean up spaces, and encode the command. We encode the
            # command here to be able to pass it to a python script to run later without
            # dealing with any substitution or evaluation within a shell. Thus, this is
            # akin to the SubprocessRunner also running commands. It's a bit complicated,
            # but I promise that it's much better than the alternative
            command = command.replace('\n', ' ')
            command = ' '.join(command.split())
            command_encoded = urllib.parse.quote(command)

            # Script used to decode the command as described above
            hpc_run = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'hpc_run.py')

            # Special logic for when we're running with apptainer, in which case
            # we need to manipulate the command like such
            # Original command: <mpiexec ...> </path/to/binary ...>
            # New command: <mpiexec ...> apptainer exec /path/to/image '</path/to/binary ...>'
            APPTAINER_CONTAINER = os.environ.get('APPTAINER_CONTAINER')
            if APPTAINER_CONTAINER:
                job_command = mpi_prefix

                # The root filesystem path that we're in so that we can be sure to bind
                # it into the container, if not already set
                if self.options.hpc_apptainer_bindpath:
                    bindpath = self.options.hpc_apptainer_bindpath
                else:
                    bindpath = '/' + os.path.abspath(tester.getTestDir()).split(os.path.sep)[1]
                submission_env['VARS']['APPTAINER_BINDPATH'] = bindpath + ',${APPTAINER_BINDPATH}'

                serial_command = shlex.join(self.app_exec_prefix + self.app_exec_suffix)

                # Append the apptainer command along with the command to be ran
                job_command += f"{serial_command} {hpc_run} {command_encoded}"
            # Not in apptainer, so we can just use the escaped command as is
            else:
                job_command = f'{hpc_run} {command_encoded}'

            submission_env['COMMAND'] = job_command

            # The output files that we're expected to generate so that the
            # HPC job can add a terminator for them so that we can verify
            # they are complete on the executing host
            additional_output = [result_file]
            for file in tester.getOutputFiles(options):
                additional_output.append(os.path.join(tester.getTestDir(), file))
            # This is a bash array, need to wrap each entry in double quotes
            additional_output_wrapped = []
            for entry in additional_output:
                additional_output_wrapped.append(f'"{entry}"')
            submission_env['ADDITIONAL_OUTPUT_FILES'] = ' '.join(additional_output_wrapped)

            # Let the derived scheduler add additional variables
            self.augmentJobSubmission(submission_env)

            # Build the script
            jinja_env = jinja2.Environment()
            definition_template = jinja_env.from_string(self.submission_template)
            jinja_env.trim_blocks = True
            jinja_env.lstrip_blocks = True
            script = definition_template.render(**submission_env)

            # Write the script
            open(submission_script, 'w').write(script)

            # Submission command. Here we have a simple bash loop
            # that will try to wait for the file if it doesn't exist yet
            submission_command = self.getHPCSubmissionCommand()
            cmd = [f'cd {tester.getTestDir()}',
                f'FILE="{submission_script}"',
                    'for i in {1..40}',
                        'do if [ -e "$FILE" ]',
                        f'then {self.getHPCSubmissionCommand()} $FILE',
                                'exit $?',
                            'else sleep 0.25',
                        'fi',
                    'done',
                    'exit 1']
            cmd = '; '.join(cmd)

            # Do the submission; this is thread safe
            exit_code, result, full_cmd = self.callHPC(self.CallHPCPoolType.submit, cmd, num_retries=5)

            # Start the queued timer if needed
            if not hold:
                job.timer.start('hpc_queued')

            # Set what we've ran for this job so that we can
            # potentially get the context in an error
            tester.setCommandRan(full_cmd)

            # Nonzero return code
            if exit_code != 0:
                raise self.CallHPCException(self, f'{submission_command} failed', full_cmd, result)

            # Set the HPC job state
            hpc_job.id = self.parseHPCSubmissionJobID(result)
            hpc_job.command = job_command
            hpc_job.state = hpc_job.State.held if hold else hpc_job.State.queued

            # Job has been submitted, so set it as queued
            # Here we append job_id if the ID is just a number so that it's more
            # obvious what it is
            job.addCaveats(self.jobCaveat(hpc_job))

            # Print the job as it's been submitted
            job_status = job.hold if hold else job.queued
            self.setAndOutputJobStatus(hpc_job.job, job_status, caveats=True)

        return hpc_job

    def queueJob(self, job):
        """
        Method for queuing a Job to start.

        Should be called from within the HPCRunner to get a job going.

        If the job is not submitted yet, it will submit it in a
        non-held state. If the job is submitted but held, it will
        release the job.
        """
        # See if the job has been submitted yet in the background
        with self.hpc_jobs_lock:
            hpc_job = self.hpc_jobs.get(job.getID())

            # If the updater hasn't been started yet, start it.
            # We do this here because it's locked within hpc_jobs_lock
            # and it means that we won't start looking for jobs until
            # we have at least one job
            if not self.hpc_jobs_updater:
                self.hpc_jobs_updater = threading.Thread(target=self._updateHPCJobs)
                self.hpc_jobs_updater.start()

        # Job has not been submitted yet, so submit it in non-held state
        if hpc_job is None:
            return self.submitJob(job, False)

        # Job has been submitted but is held, so queue it
        with hpc_job.getLock():
            if hpc_job.state == hpc_job.State.held:
                if self.options.hpc_no_hold:
                    raise Exception('Job should not be held with holding disabled')

                cmd = f'{self.getHPCQueueCommand()} {hpc_job.id}'
                exit_code, result, full_cmd = self.callHPC(self.CallHPCPoolType.queue, cmd, num_retries=5)
                if exit_code != 0:
                    try:
                        self.killHPCJob(hpc_job, lock=False) # already locked
                    except:
                        pass
                    raise self.CallHPCException(self, f'{cmd} failed', full_cmd, result)

                # Start the timer now that we've queued it
                hpc_job.job.timer.start('hpc_queued')

                self.setHPCJobQueued(hpc_job)

            return hpc_job

    def augmentJobSubmission(self, submission_env):
        """
        Entry point for derived schedulers to append to the
        submission environment, which is used to populate
        the submission jinja template.
        """
        return

    def _updateHPCJobs(self):
        """
        Function that is called in a separate thread to update the job
        status given some interval.
        """
        # We want to allow failure to happen once, just not twice in a row.
        # This is a good sanity check for when occasionally the login
        # node doesn't respod as expected
        update_jobs_failed = False

        try:
            while True:
                # Here we want to store our own list to these objects
                # so that we don't hold onto the lock while we work
                # on each job individually
                with self.hpc_jobs_lock:
                    hpc_jobs = [x for x in self.hpc_jobs.values()]

                # Get all of the HPC jobs that are currently active
                active_states = [HPCJob.State.queued, HPCJob.State.running]
                active_hpc_jobs = [x for x in hpc_jobs if x.getState() in active_states]

                # Helper for splitting a list into chunks. We won't update
                # everything together because PBS is particularly bad
                # at processing the status for a ton of jobs at once...
                def in_chunks(l):
                    N = self.update_hpc_jobs_chunk_size
                    for i in range(0, len(l), N):
                        yield l[i:i + N]

                # Whether or not all of the updates suceeded
                success = True

                # Process a subset of jobs at a time
                for chunked_hpc_jobs in in_chunks(active_hpc_jobs):
                    # Returns whether or not it failed
                    if not self.updateHPCJobs(chunked_hpc_jobs):
                        success = False

                # At least one of the updates failed; allow this to
                # happen only once
                if not success:
                    if update_jobs_failed:
                        self.triggerErrorState()
                        print('ERROR: Failed to get HPC job status')
                        return
                    update_jobs_failed = True
                else:
                    update_jobs_failed = False

                # Update on the interval requested, but also make sure
                # that we're still running
                poll_time = 0.1
                for i in range(int(self.hpc_jobs_update_interval / poll_time)):
                    if not self.isRunning():
                        return
                    time.sleep(poll_time)
        except:
            self.triggerErrorState()
            raise

    def updateHPCJobs(self, hpc_jobs):
        """
        Updates the underlying jobs.

        Should be overridden and should return True or False
        depending on whether or not the update succeeded.

        Should use setHPCJobRunning() and setHPCJobDone()
        to trigger changes in HPC job state.
        """
        raise Exception('Unimplemented updateHPCJobs()')

    def setHPCJobRunning(self, hpc_job, start_time):
        """
        Sets the given HPC job as running.

        Should be called within a lock for the given HPCJob.

        This should be called within the overridden updateHPCJobs() to
        set a HPCJob as running.
        """
        job = hpc_job.job
        timer = job.timer

        # This is currently thread safe because we only ever change
        # it within updateJobs(), which is only ever executed serially
        # within the thread the calls _updateHPCJobs()
        hpc_job.state = hpc_job.State.running

        # The job is no longer queued as of when it started
        if timer.hasTime('hpc_queued'):
            queued_start_time = timer.startTime('hpc_queued')
            # This can happen on slurm in < 1s, which could give us negatives
            if start_time < queued_start_time:
                timer.stop('hpc_queued', queued_start_time)
            else:
                timer.stop('hpc_queued', start_time)
        # The runner job (actual walltime for the exec) as of when it started
        timer.start('runner_run', start_time)

        # Print out that the job is now running
        self.setAndOutputJobStatus(hpc_job.job, hpc_job.job.running, caveats=True)

    def setHPCJobQueued(self, hpc_job):
        """
        Sets the given HPC job as being queued.

        Should be called within a lock for the given HPCJob.

        This can be used when the HPC scheduler re-schedules the job.

        This should be called within the overridden updateHPCJobs().
        """
        # Guard against setting this as requeued multiple times
        if hpc_job.state == hpc_job.State.queued:
            return
        hpc_job.state = hpc_job.State.queued

        # Print out that the job is queued again
        self.setAndOutputJobStatus(hpc_job.job, hpc_job.job.queued, caveats=True)

    def setHPCJobDone(self, hpc_job, exit_code, end_time):
        """
        Sets the given HPC job as done.

        This should be called within the overridden updateHPCJobs(),
        within a thread lock for that HPCJob.
        """
        job = hpc_job.job

        hpc_job.state = hpc_job.State.done
        hpc_job.exit_code = exit_code

        # The runner job (actual walltime for the exec) ends when it stopped
        if job.timer.hasTime('runner_run'):
            job.timer.stop('runner_run', end_time)

        # We've actually ran something now that didn't fail, so update
        # the command to what was ran there
        if not job.isError():
            job.getTester().setCommandRan(hpc_job.command)

    def buildRunner(self, job, options):
        from TestHarness.runners.HPCRunner import HPCRunner
        return HPCRunner(job, options, self)

    def augmentJobs(self, jobs):
        super().augmentJobs(jobs)

        # Augment only jobs that are to be ran
        for job in jobs:
            if job.isHold():
                # If a job has its default time, double it. We grant a
                # little more time to small jobs on HPC due to slower IO, etc
                tester = job.getTester()
                max_time = tester.getMaxTime()
                if max_time == tester.getDefaultMaxTime():
                    tester.setMaxTime(max_time * 2)

                # Add the Job to the pool to be submitted as a job in
                # a held state. We do this as early as possible so that
                # we can get a better priority in the HPC queue. This
                # is an asynchronous call so it will happen later when
                # available. If the Job actually runs before we have
                # a chance to get to this in the pool, when it finally
                # executes in the pool, it will do nothing because the
                # HPCJob will already exist.
                if not self.options.hpc_no_hold and not self.options.dry_run:
                    self.submit_job_pool.apply_async(self.submitJob, (job, True,))

    def killHPCJob(self, hpc_job, lock=True):
        """
        Kills the given HPCJob if it is in a state to be killed.
        """
        with hpc_job.getLock() if lock else contextlib.suppress():
            if hpc_job.state in [hpc_job.State.killed, hpc_job.State.done]:
                return
            job_id = hpc_job.id
            hpc_job.state = hpc_job.State.killed

        # Don't care about whether or not this failed
        self.callHPC(self.CallHPCPoolType.kill, f'{self.getHPCCancelCommand()} {job_id}')

    def killHPCJobs(self, functor):
        """
        Kills the HPC jobs the meet the criteria of the functor.

        The functor should take a single object, the HPCJob, and
        should return a bool stating whether or not to kill that job.
        """
        job_ids = []
        with self.hpc_jobs_lock:
            for hpc_job in self.hpc_jobs.values():
                with hpc_job.getLock():
                    if functor(hpc_job):
                        job_ids.append(hpc_job.id)

        if job_ids:
            self.callHPC(self.CallHPCPoolType.kill, f'{self.getHPCCancelCommand()} {" ".join(job_ids)}')

        return len(job_ids)

    def killRemaining(self, keyboard=False):
        """Kills all currently running HPC jobs"""
        running_states = [HPCJob.State.killed, HPCJob.State.done]
        functor = lambda hpc_job: hpc_job is not None and hpc_job.state not in running_states
        killed_jobs = self.killHPCJobs(functor)
        if keyboard and killed_jobs:
            print(f'\nAttempted to kill remaining {killed_jobs} HPC jobs...')
        super().killRemaining(keyboard)

    def getHPCSchedulerName(self):
        """
        Returns the name of the HPC scheduler in a simple shorthand.

        Used to produce files with a prefix of the scheduler type, i.e.,
        pbs_something or slurm_something.

        Should be overridden.
        """
        raise Exception('Unimplemented getHPCSchedulerName()')

    def getHPCSubmissionCommand(self):
        """
        Returns command used for submitting jobs.

        Should be overridden.
        """
        raise Exception('Unimplemented getHPCSubmissionCommand()')

    def getHPCQueueCommand(self):
        """
        Returns command used for submitting jobs.

        Should be overridden.
        """
        raise Exception('Unimplemented getHPCQueueCommand()')

    def getHPCCancelCommand(self):
        """
        Returns comamnd used for cancelling jobs.

        Should be overridden.
        """
        raise Exception('Unimplemented getHPCCancelCommand()')

    def getHPCJobIDVariable(self):
        """
        Returns the environment variable name that contains the job ID
        when within a job (i.e., on a compute node).

        Should be overridden.
        """
        raise Exception('Unimplemented getHPCJobIDVariable()')

    def parseHPCSubmissionJobID(self, result):
        """
        Returns the job ID from the result of the submission command
        (from qsub or sbatch).

        Should be overridden.
        """
        raise Exception('Unimplemented parseHPCSubmissionJobID()')

    @staticmethod
    def getHPCJobName(job) -> str:
        """Gets the name of the HPC job given a tester

        PBS doesn't like ":" or "/", hence changing them to "."
        """
        return job.getTestName().replace(':', '.').replace('/', '.')

    def getHPCJobOutputPath(self, job):
        """Gets the absolute path for stdout/stderr for a HPC job"""
        return job.getOutputPathPrefix() + '.hpc_out.txt'

    def getHPCJobResultPath(self, job):
        """Gets the absolute path for the result (exit code, walltime) for a HPC job"""
        return job.getOutputPathPrefix() + '.hpc_result'

    def getHPCJobSubmissionPath(self, job):
        """Gets the aboslute path for the qsub script for a HPC job"""
        return job.getOutputPathPrefix() + '.hpc_submit'

    @staticmethod
    def getOutputEndingComment(job_id) -> str:
        """
        Get the ending comment that is applied to all output files
        that are read in order to verify that the files are fully
        synced when reading during postprocessing.
        """
        return f'TESTHARNESS RUNHPC FILE TERMINATOR FOR {job_id}'

    @staticmethod
    def parseMPICommand(command) -> str:
        """
        Helper that splits out the mpi command from a given command, if any
        """
        find_mpi = re.search(r'^(\s+)?(mpiexec|mpirun)(\s+-(n|np)\s+\d+)?(\s+)?', command)
        if find_mpi is not None:
            return find_mpi.group(0)
        return None

    @staticmethod
    def setHPCJobError(hpc_job, message, output=None):
        """
        Helper for setting an error within a HPC job.

        Should be used within the derived classes updateHPCJobs().
        """
        job = hpc_job.job
        job.setStatus(job.error, message)
        if output:
            job.appendOutput(util.outputHeader(f'Job {hpc_job.id} {output}'))

    def waitFinish(self):
        super().waitFinish()

        # Kill the remaining jobs that are held, which would exist if things
        # fail and jobs that we pre-submitted were skipped due to a failed
        # dependency above them
        functor = lambda hpc_job: hpc_job.state == hpc_job.State.held
        self.killHPCJobs(functor)

    def appendStats(self):
        timer_keys = ['hpc_queued', 'hpc_wait_output']
        times = {}
        for key in timer_keys:
            times[key] = []
        num_resubmit = 0

        for hpc_job in self.hpc_jobs.values():
            timer = hpc_job.job.timer
            num_resubmit += hpc_job.num_resubmit
            for key in timer_keys:
                if timer.hasTotalTime(key):
                    times[key].append(timer.totalTime(key))

        averages = {}
        for key, values in times.items():
            averages[key] = statistics.mean(values) if values else 0

        stats = super().appendStats()
        stats.update({'hpc_time_queue_average': averages['hpc_queued'],
                      'hpc_time_wait_output_average': averages['hpc_wait_output'],
                      'hpc_num_resubmitted': num_resubmit})
        return stats

    def appendResultFooter(self, stats):
        result = f'Average queue time {stats["hpc_time_queue_average"]:.1f} seconds, '
        result += f'average output wait time {stats["hpc_time_wait_output_average"]:.1f} seconds, '
        result += f'{stats["hpc_num_resubmitted"]} jobs resubmitted.'
        return result

    def appendResultFileHeader(self):
        entry = {'scheduler': self.options.hpc,
                 'hosts': self.options.hpc_host if isinstance(self.options.hpc_host, list) else [self.options.hpc_host]}
        return {'hpc': entry}

    def appendResultFileJob(self, job):
        hpc_job = self.hpc_jobs.get(job.getID())
        if not hpc_job:
            return {'hpc': None}
        entry = {'id': hpc_job.id,
                 'submission_script': self.getHPCJobSubmissionPath(job)}
        return {'hpc': entry}

    def callHPCShouldRetry(self, pool_type, result: str):
        """
        Entry point for a derived scheduler class to tell us if we can
        retry a command given a failure with a certain result.
        """
        return False

    def setupRunEnvironment(self, harness):
        """
        Sets up the run environment for all HPC jobs
        """
        hpc_cluster = harness.queryHPCCluster(self.ssh_hosts[0])

        # HPC containerized module that we're in, if any
        module_name = os.environ.get('CONTAINER_MODULE_NAME')
        # Container that we're in, if any
        apptainer_container = os.environ.get('APPTAINER_CONTAINER')

        # Base submission environment
        submit_env = {# Name of the scheduler
                      'SCHEDULER_NAME': self.getHPCSchedulerName(),
                      # Project to submit to within the cluster scheduler
                      'PROJECT': self.options.hpc_project,
                      # Ending comment for output files
                      'ENDING_COMMENT': self.getOutputEndingComment(f'${self.getHPCJobIDVariable()}'),
                      # Env var on the compute node that contains the HPC scheduler JOB id
                      'JOB_ID_VARIABLE': self.getHPCJobIDVariable(),
                      # Modules to load
                      'LOAD_MODULES': [],
                      # Environment variables to set before loading the modules
                      'PRE_MODULE_VARS': {},
                      # Environment variables to set after loading the modules
                      'VARS': {},
                      # The host the HPC job was submitted from
                      'SUBMITTED_HOSTNAME': socket.gethostname(),
                      # Whether or not we're using apptainer
                      'USING_APPTAINER': apptainer_container is not None}

        # The prefix and suffix to wrap around what we're actually
        # running within the HPC job
        app_exec_prefix = []
        app_exec_suffix = []

        # --hpc-pre-source contents
        if self.options.hpc_pre_source:
            submission_env['PRE_SOURCE_FILE'] = self.options.hpc_pre_source
            submission_env['PRE_SOURCE_CONTENTS'] = self.source_contents

        # If running on INL HPC, minimize the bindpath; this is a configuration
        # option for the moose-dev-container module on INL HPC
        if hpc_cluster is not None:
            submit_env['PRE_MODULE_VARS']['MOOSE_DEV_CONTAINER_MINIMAL_BINDPATH'] = '1'

        # Pass apptainer options
        if self.options.hpc_apptainer_no_home:
            submit_env['VARS']['APPTAINER_NO_HOME'] = '1'

        # Add MOOSE's pythonpath
        moose_python = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)), '../..'))
        submit_env['VARS']['PYTHONPATH'] = moose_python + ':${PYTHONPATH}'

        # We're in a loaded container module on INL HPC, use that environment
        # and the associated <container>-exec command to run apptainer
        if module_name:
            assert apptainer_container is not None
            module_version = os.environ['CONTAINER_MODULE_VERSION']
            module_exec = os.environ['CONTAINER_MODULE_EXEC']
            required_modules = os.environ.get('CONTAINER_MODULE_REQUIRED_MODULES', '').split(' ')
            modules = required_modules + [f'{module_name}/{module_version}']
            submit_env['LOAD_MODULES'] = modules
            app_exec_prefix = [module_exec]
        # We're in a container without the container module environment, use
        # a direct apptainer exec command
        elif apptainer_container:
            # If on INL HPC, use the wrapped environment
            if hpc_cluster is not None:
                submit_env['LOAD_MODULES'] = hpc_cluster.apptainer_modules
            app_exec_prefix = ['apptainer', 'exec']
            app_exec_suffix = [apptainer_container]

        if submit_env['LOAD_MODULES']:
            print(f'INFO: Using modules "{" ".join(submit_env["LOAD_MODULES"])}" for HPC environment')
        if app_exec_prefix:
            exec_combined = app_exec_prefix + app_exec_suffix
            print(f'INFO: Using "{" ".join(exec_combined)}" as HPC execution wrapper')

        return submit_env, app_exec_prefix, app_exec_suffix
