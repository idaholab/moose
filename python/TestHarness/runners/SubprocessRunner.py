#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import copy, os, platform, subprocess, shlex, time, threading
from tempfile import SpooledTemporaryFile
from signal import SIGTERM
from TestHarness.runners.Runner import Runner

class SubprocessRunner(Runner):
    """
    Runner that spawns a local subprocess.
    """
    def __init__(self, job, options):
        Runner.__init__(self, job, options)

        # The output file handler
        self.outfile = None
        # The error file handler
        self.errfile = None
        # The underlying subprocess
        self.process = None

        # List of resource usage over time, if enabled
        self.resource_usage = None
        # Lock for accessing self.resource_usage
        self.resource_usage_lock = threading.Lock()

    def spawn(self, timer):
        tester = self.job.getTester()
        use_shell = tester.specs["use_shell"]
        cmd = tester.getCommand(self.options)
        tester.setCommandRan(cmd)

        # Split command into list of args to be passed to Popen
        if not use_shell:
            cmd = shlex.split(cmd)

        self.process = None
        self.outfile = SpooledTemporaryFile(max_size=1000000) # 1M character buffer
        self.errfile = SpooledTemporaryFile(max_size=100000)  # 100K character buffer

        process_args = [cmd]
        process_kwargs = {'stdout': self.outfile,
                          'stderr': self.errfile,
                          'close_fds': False,
                          'shell': use_shell,
                          'cwd': tester.getTestDir()}
        # On Windows, there is an issue with path translation when the command is passed in
        # as a list.
        if platform.system() == "Windows":
            process_kwargs['creationflags'] = subprocess.CREATE_NEW_PROCESS_GROUP
        else:
            process_kwargs['preexec_fn'] = os.setsid

        # Special logic for openmpi runs
        if tester.hasOpenMPI():
            process_env = os.environ.copy()

            # Don't clobber state
            process_env['OMPI_MCA_orte_tmpdir_base'] = self.job.getTempDirectory().name
            # Allow oversubscription for hosts that don't have a hostfile
            process_env['PRTE_MCA_rmaps_default_mapping_policy'] = ':oversubscribe'

            process_kwargs['env'] = process_env

        try:
            self.process = subprocess.Popen(*process_args, **process_kwargs)
        except Exception as e:
            raise Exception('Error in launching a new task') from e

        timer.start('runner_run')

    def wait(self, timer):
        # If psutil is available, track resources over time for this job.
        # This replaces the traditional self.process.wait() call with
        # a loop that polls for resources in an interval
        try:
            import psutil
        except ModuleNotFoundError:
            pass
        else:
            poll_time = 0.1 # sec
            psutil_process = psutil.Process(self.process.pid)
            self.resource_usage = []
            start_poll_time = time.time()
            last_poll_time = None
            while True:
                if last_poll_time is not None:
                    sleep_time = poll_time - (time.time() - last_poll_time)
                    if sleep_time > 0:
                        time.sleep(sleep_time)
                last_poll_time = time.time()

                # Get RSS memory usage for the top level process
                # and all of its children
                try:
                    total_memory = psutil_process.memory_info().rss
                    children = psutil_process.children(recursive=True)
                except psutil.NoSuchProcess:
                    break
                for child in children:
                    try:
                        total_memory += child.memory_info().rss
                    except psutil.NoSuchProcess:
                        pass

                # Store the usage and make sure that the usage
                # doesn't oversubscribe any requirements
                usage = self.ResourceUsage(time=last_poll_time - start_poll_time,
                                            mem_bytes=total_memory)
                self.checkResourceUsage(usage)
                with self.resource_usage_lock:
                    self.resource_usage.append(usage)

                # Process has ended
                if self.process.poll() is not None:
                    break

        self.process.wait()
        timer.stop('runner_run')

        self.exit_code = self.process.poll()
        self.process = None

        # Load combined output
        for file in [self.outfile, self.errfile]:
            file.flush()
            output = self.readOutput(file)
            file.close()

            # For some reason openmpi will append a null character at the end
            # when the exit code is nonzero. Not sure why this is... but remove
            # it until we figure out what's broken
            if file == self.errfile and self.exit_code != 0 \
                and self.job.getTester().hasOpenMPI() and len(output) > 2 \
                and output[-3:] in ['\n\0\n', '\n\x00\n']:
                output = output[:-3]

            self.getRunOutput().appendOutput(output)

    def getResourceUsage(self):
        with self.resource_usage_lock:
            if self.resource_usage is None:
                return None
            return copy.deepcopy(self.resource_usage)

    def kill(self):
        if self.process is not None:
            try:
                if platform.system() == "Windows":
                    from distutils import spawn
                    if spawn.find_executable("taskkill"):
                        subprocess.call(['taskkill', '/F', '/T', '/PID', str(self.process.pid)])
                    else:
                        self.process.terminate()
                else:
                    pgid = os.getpgid(self.process.pid)
                    os.killpg(pgid, SIGTERM)
            except OSError: # Process already terminated
                pass

    def sendSignal(self, signal):
        # process.poll() returns the process's exit code if it has completed,
        # and None if it is still running. This acts as a safety precaution
        # against an infinite loop; this will always close.
        while self.process.poll() is None:

            # tell() gives the current position in the file. If it is greater
            # than zero, the binary has started running and writing output. If
            # the output is blank, the moose_test binary hasn't actually started
            # doing anything yet. If so, sleep briefly and check again.
            if not self.outfile.tell():
                time.sleep(0.05)

            # If the output isn't blank, then we finally send the signal and exit the loop
            else:
                os.kill(self.process.pid, signal)
                break
