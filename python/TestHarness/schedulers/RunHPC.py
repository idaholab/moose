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
import threading, os, re, sys, datetime, shlex, socket, threading, time, urllib
import paramiko
import jinja2
from multiprocessing.pool import ThreadPool

class HPCJob:
    """
    Structure that represents the cached information about an HPC job
    """
    def __init__(self, job, id, command):
        # The underlying Job (only set on init, _should_ be thread safe)
        self.job = job
        # The job identifier (only set on init, _should_ be thread safe)
        self.id = id
        # The command that was ran within the job
        self.command = command
        # Whether or not this job is done; here done doesn't mean if it
        # was successful or not, just if it is not running/queued anymore
        self.done = False
        # The exit code of the command that was ran (if any)
        self.exit_code = None
        # Whether or not this job was killed; used so what we don't
        # bother killing a job multiple times
        self.killed = False
        # Whether or not the job is currently running
        self.running = False
        # Lock for accessing this object
        self.lock = threading.Lock()

    def getLock(self):
        """
        Gets the lock for this object.
        """
        return self.lock

    def set(self, **kwargs):
        """
        Thread-safe setter.
        """
        with self.getLock():
            for key, value in kwargs.items():
                setattr(self, key, value)

    def getExitCode(self):
        """
        Gets the thread-safe exit code.

        This exit code is what is read by the HPCRunner,
        which means that it needs to be locked as we're
        also updating it at the same time.
        """
        with self.getLock():
            return self.exit_code

    def getRunning(self):
        """
        Gets the thread-safe running state.
        """
        with self.getLock():
            return self.running

    def getKilled(self):
        """
        Gets the thread-safe killed state.
        """
        with self.getLock():
            return self.killed

    def getDone(self):
        """
        Gets the thread-safe done state.
        """
        with self.getLock():
            return self.done

class RunHPC(RunParallel):
    """
    Base scheduler for jobs that are ran on HPC.
    """
    def __init__(self, harness, params):
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
        # How often to poll for status updates in getHPCJob()
        self.hpc_jobs_update_interval = 10
        # Map of Job -> HPCJob
        self.hpc_jobs = {}
        # The thread that will update the HPCJobs
        self.hpc_jobs_updater = None

        # The jump hostname for running commands, if any
        self.ssh_hosts = self.options.hpc_host
        # The SSH key to use for connections
        self.ssh_key_filenames = None
        # The pool of processes for running threaded SSH comments
        self.ssh_pool = None
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
            self.ssh_pool = ThreadPool(processes=5)
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

            # Make sure that we can connect up front
            self.callHPC('hostname')

        if os.environ.get('APPTAINER_CONTAINER'):
            if not self.ssh_hosts:
                print('ERROR: --hpc-host must be set when using HPC jobs within apptainer')
                sys.exit(1)
            if not self.options.hpc_pre_source:
                default_pre_source =  os.path.join(os.path.abspath(os.path.dirname(__file__)), 'hpc_source')
                self.options.hpc_pre_source = default_pre_source
                print(f'INFO: Setting --hpc-pre-source={default_pre_source}')
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
        self.source_contents = None
        if self.options.hpc_pre_source:
            self.source_contents = open(self.options.hpc_pre_source, 'r').read()

        # Load the submission template
        template_path = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'hpc_template')
        self.submission_template = open(template_path, 'r').read()

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

        Should only be used via apply with the self.ssh_pool.
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

    def callHPC(self, command):
        """
        Wrapper for calling a HPC command (qsub, qstat, etc) that supports
        SSH-ing to another host as needed when calling from within apptainer
        """
        if not self.ssh_hosts:
            raise Exception('HPC not currently supported outside of a container')

        return self.ssh_pool.apply(self._callSSH, (command,))

    def getJobSlots(self, job):
        # Jobs only use one slot because they are ran externally
        return 1

    def availableSlots(self, params):
        # Support managing 250 HPC jobs concurrently
        return 250, False

    def submitJob(self, job):
        """
        Method for submitting an HPC job for the given Job.

        Returns the job's ID and the command to be ran in the job.
        """
        tester = job.getTester()
        options = self.options

        submission_script = self.getHPCJobSubmissionPath(job)
        output_file = self.getHPCJobOutputPath(job)

        # Clean these two files
        for file in [submission_script, output_file]:
            if os.path.exists(file):
                os.remove(file)

        # Add MOOSE's python path for python scripts
        moose_python = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)), '../..'))

        # Start building the jinja environment for the submission script
        submission_env = {'SCHEDULER_NAME': self.getHPCSchedulerName(),
                          'NAME': self.getHPCJobName(job),
                          'CWD': tester.getTestDir(),
                          'OUTPUT': output_file,
                          'SUBMISSION_SCRIPT': submission_script,
                          'WALLTIME': str(datetime.timedelta(seconds=tester.getMaxTime())),
                          'PROJECT': self.options.hpc_project,
                          'TEST_SPEC': tester.getSpecFile(),
                          'TEST_NAME': tester.getTestNameShort(),
                          'SUBMITTED_HOSTNAME': socket.gethostname(),
                          'MOOSE_PYTHONPATH': moose_python,
                          'NUM_PROCS': tester.getProcs(options),
                          'NUM_THREADS': tester.getThreads(options),
                          'ENDING_COMMENT': self.getOutputEndingComment(f'${self.getHPCJobIDVariable()}'),
                          'JOB_ID_VARIABLE': self.getHPCJobIDVariable(),
                          'PLACE': self.options.hpc_place}
        if self.options.hpc_pre_source:
            submission_env['SOURCE_FILE'] = options.hpc_pre_source
        if self.source_contents:
            submission_env['SOURCE_CONTENTS'] = self.source_contents

        # Get the unescaped command
        command = tester.getCommand(options)

        # Parse out the mpi command from the command if we're running in apptainer.
        # We do this before any of the other escaping
        APPTAINER_CONTAINER = os.environ.get('APPTAINER_CONTAINER')
        apptainer_command_prefix = ''
        if APPTAINER_CONTAINER:
            mpi_command = self.parseMPICommand(command)
            if mpi_command:
                apptainer_command_prefix = mpi_command
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
        if APPTAINER_CONTAINER:
            job_command = apptainer_command_prefix

            # The root filesystem path that we're in so that we can be sure to bind
            # it into the container, if not already set
            if self.options.hpc_apptainer_bindpath:
                bindpath = self.options.hpc_apptainer_bindpath
            else:
                bindpath = '/' + os.path.abspath(tester.getTestDir()).split(os.path.sep)[1]
            # The apptainer command that will get sandwiched in the middle
            apptainer_command = ['apptainer', 'exec', '-B', bindpath]
            if self.options.hpc_apptainer_no_home:
                apptainer_command.append('--no-home')
            apptainer_command.append(APPTAINER_CONTAINER)
            apptainer_command = shlex.join(apptainer_command)

            # Append the apptainer command along with the command to be ran
            job_command += f"{apptainer_command} {hpc_run} {command_encoded}"

            # Set that we're using apptainer
            submission_env['USING_APPTAINER'] = '1'
        # Not in apptainer, so we can just use the escaped command as is
        else:
            job_command = f'{hpc_run} {command_encoded}'

        submission_env['COMMAND'] = job_command

        # The output files that we're expected to generate so that the
        # HPC job can add a terminator for them so that we can verify
        # they are complete on the executing host
        additional_output = []
        for file in tester.getOutputFiles(options):
            additional_output.append(f'"{os.path.join(tester.getTestDir(), file)}"')
        submission_env['ADDITIONAL_OUTPUT_FILES'] = ' '.join(additional_output)

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
        exit_code, result, full_cmd = self.callHPC(cmd)

        # Set what we've ran for this job so that we can
        # potentially get the context in an error
        tester.setCommandRan(full_cmd)

        # Nonzero return code
        if exit_code != 0:
            raise self.CallHPCException(self, f'{submission_command} failed', full_cmd, result)

        # Parse the job ID from the command
        job_id = self.parseHPCSubmissionJobID(result)

        # Job has been submitted, so set it as queued
        # Here we append job_id if the ID is just a number so that it's more
        # obvious what it is
        job.addCaveats(f'job={job_id}' if job_id.isdigit() else job_id)

        # Setup the job in the status map
        with self.hpc_jobs_lock:
            if job in self.hpc_jobs:
                raise Exception('Job has already been submitted')
            hpc_job = HPCJob(job, job_id, job_command)
            self.hpc_jobs[job] = hpc_job

            # Set the job as queued and print out that it is queued
            self.setHPCJobQueued(hpc_job)

            # If the updater hasn't been started yet, start it.
            # We do this here because it's locked within hpc_jobs_lock
            # and it means that we won't start looking for jobs until
            # we have at least one job
            if not self.hpc_jobs_updater:
                self.hpc_jobs_updater = threading.Thread(target=self._updateHPCJobs)
                self.hpc_jobs_updater.start()

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
                with self.hpc_jobs_lock:
                    active_hpc_jobs = [x for x in self.hpc_jobs.values() if not x.done]
                    if active_hpc_jobs:
                        success = self.updateHPCJobs(active_hpc_jobs)
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

    def updateHPCJobs(self, active_hpc_jobs):
        """
        Updates the underlying jobs.

        Should be overridden and should return True or False
        depending on whether or not the update succeeded.

        Should use setHPCJobRunning() and setHPCJobDone()
        to trigger changes in HPC job state.
        """
        raise Exception('Unimplemented updateHPCJobs()')

    def setHPCJobRunning(self, hpc_job):
        """
        Sets the given HPC job as running.

        This should be called within the overridden updateHPCJobs().
        """
        # This is currently thread safe because we only ever change
        # it within updateJobs(), which is only ever executed serially
        # within the thread the calls _updateHPCJobs()
        hpc_job.set(running=True)
        # Print out that the job is now running
        self.setAndOutputJobStatus(hpc_job.job, hpc_job.job.running, caveats=True)

    def setHPCJobQueued(self, hpc_job):
        """
        Sets the given HPC job as being queued.

        This can be used when the HPC scheduler re-schedules the job.

        This should be called within the overridden updateHPCJobs().
        """
        # Guard against setting this as requeued multiple times
        if hpc_job.job.getStatus() == hpc_job.job.queued:
            return

        # This is currently thread safe because we only ever change
        # it within updateJobs(), which is only ever executed serially
        # within the thread the calls _updateHPCJobs()
        hpc_job.set(running=False)
        # Print out that the job is queued again
        self.setAndOutputJobStatus(hpc_job.job, hpc_job.job.queued, caveats=True)

    def setHPCJobDone(seflf, hpc_job, exit_code):
        """
        Sets the given HPC job as done.

        This should be called within the overridden updateHPCJobs().
        """
        hpc_job.set(running=False, done=True, exit_code=exit_code)

        # We've actually ran something now that didn't fail, so update
        # the command to what was ran there
        job = hpc_job.job
        if not job.isError():
            job.getTester().setCommandRan(hpc_job.command)

    def buildRunner(self, job, options):
        from TestHarness.runners.HPCRunner import HPCRunner
        return HPCRunner(job, options, self)

    def augmentJobs(self, jobs):
        super().augmentJobs(jobs)

        # If a job has its default time, double it. We grant a little more time
        # to small jobs on HPC due to slower IO, etc
        for job in jobs:
            tester = job.getTester()
            max_time = tester.getMaxTime()
            if max_time == tester.getDefaultMaxTime():
                tester.setMaxTime(max_time * 2)

    def killJob(self, job):
        """Kills a HPC job"""
        with self.hpc_jobs_lock:
            hpc_job = self.hpc_jobs.get(job)
            if hpc_job is None or hpc_job.getDone() or hpc_job.getKilled():
                return
            job_id = hpc_job.id
            hpc_job.set(killed=True)

        # Don't care about whether or not this failed
        self.callHPC(f'{self.getHPCCancelCommand()} {job_id}')

    def killRemaining(self, keyboard=False):
        """Kills all currently running HPC jobs"""
        job_ids = []
        with self.hpc_jobs_lock:
            for hpc_job in self.hpc_jobs.values():
                if not hpc_job.getDone() and not hpc_job.getKilled():
                    job_ids.append(hpc_job.id)
                    hpc_job.set(killed=True)

        # Don't care about whether or not this failed
        self.callHPC(f'{self.getHPCCancelCommand()} {" ".join(job_ids)}')

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
        raise Exception('Unimplemented getHPCSchedulerName()')

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

    def getHPCJobOutputPathPrefix(self, job):
        """Gets the absolute path prefix for a HPC job"""
        scheduler_name = self.getHPCSchedulerName()
        return os.path.join(job.getTestDir(), f"{scheduler_name}_" + job.getTestNameShort().replace('/', '.'))

    def getHPCJobOutputPath(self, job):
        """Gets the absolute path for stdout/stderr for a HPC job"""
        return self.getHPCJobOutputPathPrefix(job) + '.out'

    def getHPCJobSubmissionPath(self, job):
        """Gets the aboslute path for the qsub script for a HPC job"""
        return self.getHPCJobOutputPathPrefix(job) + f'.{self.getHPCSubmissionCommand()}'

    @staticmethod
    def getOutputEndingComment(job_id) -> str:
        """
        Get the ending comment that is applied to all output files
        that are read in order to verify that the files are fully
        synced when reading during postprocessing.
        """
        return f'TESTHARNESS RUNHPC FILE TERMINATOR FOR {job_id}\n'

    @staticmethod
    def parseMPICommand(command) -> str:
        """
        Helper that splits out the mpi command from a given command, if any
        """
        find_mpi = re.search('^(\s+)?(mpiexec|mpirun)(\s+-(n|np)\s+\d+)?(\s+)?', command)
        if find_mpi is not None:
            return find_mpi.group(0)
        return None
