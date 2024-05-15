#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from RunParallel import RunParallel
import threading, os, re, sys, datetime, shlex, json
import paramiko
from multiprocessing.pool import ThreadPool
from timeit import default_timer as clock

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
        # The last time statues were updated in getHPCJob() (if any)
        self.hpc_jobs_status_timer = None
        # How often to poll for status updates in getHPCJob()
        self.hpc_jobs_update_interval = 10
        # Map of Job -> HPCJob
        self.hpc_jobs = {}

        # The jump hostname for running commands, if any
        self.ssh_host = self.options.queue_host
        # The SSH key to use for connections
        self.ssh_key_filename = None
        # The pool of processes for running threaded SSH comments
        self.ssh_pool = None
        # The threaded SSHClient objects, mapped by thread identifier
        self.ssh_clients = None
        # The lock for calling commands via SSH,
        self.ssh_clients_lock = None
        # Setup the jump host if provided
        if self.ssh_host:
            self.ssh_pool = ThreadPool(processes=5)
            self.ssh_clients = {}
            self.ssh_clients_lock = threading.Lock()

            # Try to find a key to use
            try:
                ssh_config = os.path.expanduser('~/.ssh/config')
                config = paramiko.SSHConfig.from_path(ssh_config).lookup(self.ssh_host)
                identityfile = config.get('identityfile')
                if identityfile is not None and len(identityfile) > 0:
                    self.ssh_key_filename = identityfile[-1]
            except:
                pass

            # Make sure that we can connect up front
            try:
                self.callHPC('hostname')
            except:
                print(f'Failed to connect to HPC host {self.ssh_host}')
                sys.exit(1)

        if os.environ.get('APPTAINER_CONTAINER'):
            if not self.ssh_host:
                print('ERROR: --hpc-host must be set when using HPC jobs within apptainer')
                sys.exit(1)
            if not self.options.queue_source_command:
                default_pre_source =  os.path.join(os.path.abspath(os.path.dirname(__file__)), 'hpc_source')
                self.options.queue_source_command = default_pre_source
                print(f'INFO: Setting --hpc-pre-source={default_pre_source}')

        if self.options.queue_source_command and not os.path.exists(self.options.queue_source_command):
            print(f'ERROR: --hpc-pre-source path {self.options.queue_source_command} does not exist')
            sys.exit(1)

        # Load the pre-source if it exists
        self.source_contents = None
        if self.options.queue_source_command:
            self.source_contents = open(self.options.queue_source_command, 'r').read()

    class HPCJob:
        """
        Structure that represents the cached information about an HPC job
        """
        def __init__(self, id, command):
            # The job identifier
            self.id = id
            # Whether or not this job is done; here done doesn't mean if it
            # was successful or not, just if it is not running/queued anymore
            self.done = False
            # The exit code of the command that was ran (if any)
            self.exit_code = None
            # The command that was ran within the submission script
            self.command = command
            # Whether or not this job was killed; used so what we don't
            # bother killing a job multiple times
            self.killed = False
            # The job state as defined by PBS
            self.state = None

    class CallHPCException(Exception):
        """
        Exception class for providing extra context for HPC submission errors
        """
        def __init__(self, run_hpc, description, command, result=None):
            message = f'{description}'
            if run_hpc.ssh_host:
                message += f' on host "{run_hpc.ssh_host}"'
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
                self.ssh_clients[process] = paramiko.SSHClient()
                self.ssh_clients[process].set_missing_host_key_policy(paramiko.AutoAddPolicy())
                self.ssh_clients[process].connect(self.ssh_host, key_filename=self.ssh_key_filename)
            return self.ssh_clients.get(process)

    def _callSSH(self, command):
        """
        Calls a SSH command.

        Should only be used via apply with the self.ssh_pool.
        """
        client = self._getSSHClient()
        try:
            _, stdout, stderr = client.exec_command(command)
        # SSH connection might have died, so try to create a new one
        except paramiko.ssh_exception.SSHException:
            try:
                client = self._getSSHClient(reconnect=True)
                _, stdout, stderr = client.exec_command(command)
            except Exception as e:
                raise RunHPC.CallHPCException(self, 'Failed to execute remote command', command) from e
        # An even worse failure happened here
        except Exception as e:
             raise RunHPC.CallHPCException(self, 'Failed to execute remote command', command) from e

        exit_code = stdout.channel.recv_exit_status()
        result = ''.join(stdout.readlines())
        if exit_code != 0:
            result += ''.join(stderr.readlines())
        return exit_code, result.rstrip()

    def callHPC(self, command):
        """
        Wrapper for calling a HPC command (qsub, qstat, etc) that supports
        SSH-ing to another host as needed when calling from within apptainer
        """
        if not self.ssh_host:
            raise Exception('HPC not currently supported outside of a container')

        return self.ssh_pool.apply(self._callSSH, (command,))

    def getJobSlots(self, job):
        # Jobs only use one slot because they are ran externally
        return 1

    def availableSlots(self, params):
        # Support managing 250 HPC jobs concurrently
        return 250, False

    class JobData:
        """
        Helper struct for storing the information to generate a job
        """
        def __init__(self):
            # The command to be ran in the job
            self.command = None
            # self.command but escaped so that it can be printed
            self.command_printable = None
            # The name of the job
            self.name = None
            # The number of procs to run the job with
            self.num_procs = None
            # The number of threads to run the job with
            self.num_threads = None
            # The combined stdout+stderr output file
            self.output_file = None
            # The additonal output files to be read (csv, exodus, etc)
            self.additional_output_files = None
            # The path to the submission script
            self.submission_script = None
            # The walltime to run the job with
            self.walltime = None

    @staticmethod
    def escapeCommand(command: str) -> str:
        """
        Escapes a command for use as a bash command
        """
        if command:
            # For the following, [1:-1] removes the additional
            # quotes that were added to wrap the command
            # General escape for bash
            command = shlex.quote(command)[1:-1]
            # Escape ' and ""
            command = json.dumps(command)[1:-1]
            # Remove newlines
            command = command.replace('\n', ' ')
            return command
        return ''

    def submitJob(self, job):
        """
        Method for submitting an HPC job for the given Job.

        Should be overridden.
        """
        tester = job.getTester()
        options = self.options

        job_data = self.JobData()

        # The submission script we're going to write to
        job_data.submission_script = self.getHPCJobSubmissionPath(job)
        # The combined stdout+stderr from the job
        job_data.output_file = self.getHPCJobOutputPath(job)
        # Clean these two files
        for file in [job_data.submission_script, job_data.output_file]:
            if os.path.exists(file):
                os.remove(file)

        # The escaped command to be ran
        command = self.escapeCommand(tester.getCommand(options))

        # Special logic for when we're running with apptainer, in which case
        # we need to manipulate the command like such
        # Original command: <mpiexec ...> </path/to/binary ...>
        # New command: <mpiexec ...> apptainer exec /path/to/image "</path/to/binary ...>"
        # This is also the reason why we have to form job_data.command_printable;
        # the extra quotes around </path/to/binary ...> need to be escaped.
        APPTAINER_CONTAINER = os.environ.get('APPTAINER_CONTAINER')
        if APPTAINER_CONTAINER:
            # Separate out the MPI command
            mpi_command = self.escapeCommand(self.parseMPICommand(command))
            # Add MPI command as the prefix and remove it from the base command
            if mpi_command:
                command_prefix = mpi_command
                command = command.replace(mpi_command, '')
            # No MPI command; nothing to do
            else:
                command_prefix = ''

            job_data.command = command_prefix
            job_data.command_printable = command_prefix

            # The root filesystem path that we're in so that we can be sure to bind
            # it into the container
            root_path = os.path.abspath(tester.getTestDir()).split(os.path.sep)[1]
            # The apptainer command that will get sandwiched in the middle
            apptainer_command = f'apptainer exec -B /{root_path} {APPTAINER_CONTAINER}'
            apptainer_command = self.escapeCommand(apptainer_command)
            # Append the apptainer command along with the command to be ran
            job_data.command += f'{apptainer_command} "{command}"'
            job_data.command_printable += f'{apptainer_command} \\"{command}\\"'
        # Not in apptainer, so we can just use the escaped command as is
        else:
            job_data.command = command
            job_data.command_printable = command

        job_data.name = self.getHPCJobName(job)
        job_data.num_procs = tester.getProcs(options)
        job_data.num_threads = tester.getThreads(options)
        job_data.walltime = str(datetime.timedelta(seconds=tester.getMaxTime()))

        # The output files that we're expected to generate so that the
        # HPC job can add a terminator for them so that we can verify
        # they are complete on the executing host
        additional_output = []
        for file in tester.getOutputFiles(options):
            additional_output.append(f'"{os.path.join(tester.getTestDir(), file)}"')
        job_data.additional_output_files = ' '.join(additional_output)

        # Let the derived class actually submit the job
        job_id = self._submitJob(job, job_data)

        # Job has been submitted, so set it as queued
        job.addCaveats(job_id)
        self.setAndOutputJobStatus(job, job.queued)

        # Setup the job in the status map
        with self.hpc_jobs_lock:
            if job in self.hpc_jobs:
                raise Exception('Job has already been submitted')
            self.hpc_jobs[job] = self.HPCJob(job_id, job_data.command)

    def _submitJob(self, job, job_data):
        """
        Submits a given job.

        Should be overridden. This is where the derived classes
        will specialize how to submit the job.
        """
        raise Exception('Unimplemented createJobScript()')

    def getHPCJob(self, job):
        """
        Gets the HPCJob object given a Job

        This will periodically update statues given a timer.
        """
        with self.hpc_jobs_lock:
            # If this is the first time seeing this job, initialize it in the list
            if job not in self.hpc_jobs:
                raise Exception('Failed to get status for unsubmitted job')

            # Only update the statues periodically as this is called across threads
            if self.hpc_jobs_status_timer is None or ((clock() - self.hpc_jobs_status_timer) > self.hpc_jobs_update_interval):
                self.updateJobs()
                self.hpc_jobs_status_timer = clock()

            return self.hpc_jobs.get(job)

    def updateJobs(self):
        """
        Updates the underlying jobs.

        Should be overridden.
        """
        raise Exception('Unimplemented updateJobs()')

    def buildRunner(self, job, options):
        from TestHarness.runners.HPCRunner import HPCRunner
        return HPCRunner(job, options, self)

    @staticmethod
    def getHPCJobName(job) -> str:
        """Gets the name of the HPC job given a tester

        PBS doesn't like ":" or "/", hence changing them to "."
        """
        return job.getTestName().replace(':', '.').replace('/', '.')

    @staticmethod
    def getHPCJobOutputPathPrefix(job):
        """Gets the absolute path prefix for a HPC job"""
        return os.path.join(job.getTestDir(), "pbs_" + job.getTestNameShort().replace('/', '.'))

    @staticmethod
    def getHPCJobOutputPath(job):
        """Gets the absolute path for stdout/stderr for a HPC job"""
        return RunHPC.getHPCJobOutputPathPrefix(job) + '.out'

    @staticmethod
    def getHPCJobSubmissionPath(job):
        """Gets the aboslute path for the qsub script for a HPC job"""
        return RunHPC.getHPCJobOutputPathPrefix(job) + '.qsub'

    @staticmethod
    def getOutputEndingComment() -> str:
        """
        Gets the text we append to the stderr+stdout file to desginate
        that it is complete
        """
        return 'TESTHARNESS RUNHPC FILE TERMINATOR'

    @staticmethod
    def parseMPICommand(command) -> str:
        """
        Helper that splits out the mpi command from a given command, if any
        """
        find_mpi = re.search('^(\s+)?(mpiexec|mpirun)(\s+-(n|np)\s+\d+)?(\s+)?', command)
        if find_mpi is not None:
            return find_mpi.group(0)
        return None
