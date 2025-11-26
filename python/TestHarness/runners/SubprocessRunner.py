#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, platform, subprocess, shlex, time
import psutil
from contextlib import suppress
from tempfile import SpooledTemporaryFile
from threading import Thread
from typing import Optional
from signal import SIGTERM
from TestHarness.runners.Runner import Runner
from TestHarness import util

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
        self.process: Optional[subprocess.Popen] = None
        # The memory checking thread, created during spawn() when -t/--timing
        self.memory_thread: Optional[Thread] = None

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

        # Setup the memory checking thread if timing enabled; joined in cleanup()
        if self.options.timing:
            self.setMaxMemory(0)
            self.memory_thread = Thread(target=self._runMemoryThread)
            self.memory_thread.start()

    def _runMemoryThread(self):
        process = self.process
        assert process is not None

        # Get the --max-memory option if set, and if so,
        # the number of procs (so that we can scale
        # memory used to per process)
        max_memory = self.options.max_memory
        procs = self.job.getTester().getProcs(self.options) if max_memory else None

        # Capture the psutil wrapper around the running process
        # so that we can query its memory usage; if this raises
        # NoSuchProcess the thread started after the process finished
        try:
            psutil_process = psutil.Process(process.pid)
        except psutil.NoSuchProcess:
            return

        # Wait for the process to finish, checking memory per step
        max_rss = 0
        while process.poll() is None:
            # Collect this process + children processes
            psutil_processes = []
            with suppress(psutil.NoSuchProcess):
                psutil_processes = [psutil_process] + psutil_process.children(
                    recursive=True
                )

            # Accumulate total resident set size for parent + children
            total_rss = 0
            for p in psutil_processes:
                with suppress(psutil.NoSuchProcess):
                    total_rss += p.memory_info().rss
            # If we have a zero value and the parent isn't
            # running anymore, we can exit
            if total_rss == 0 and not psutil_process.is_running():
                return

            # If memory has increased, set it so that it can be
            # reported live during a long-running job
            if total_rss > max_rss:
                self.setMaxMemory(total_rss)
                max_rss = total_rss

                # If --max-memory is set, make sure we're not over
                if max_memory is not None:
                    rss_per_proc = float(max_rss / procs) * 1.0e-6

                    # Over; kill the process, fail, and report
                    if rss_per_proc > max_memory:
                        self.kill()
                        message = (
                            f"Job killed: memory/slot {int(rss_per_proc)}MB "
                            f"> max {int(max_memory)}MB"
                        )
                        self.appendOutput("\n" + util.outputHeader(message, False))
                        self.job.setStatus(self.job.error, "OVER MEMORY")
                        break

            # Poll
            time.sleep(0.05)

    def wait(self, timer):
        assert self.process is not None

        self.exit_code = self.process.wait()
        timer.stop('runner_run')

        # This should have been cleared before the job started
        if self.getRunOutput().hasOutput():
            raise Exception('Runner run output was not cleared')

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

    def cleanup(self):
        """Cleanup; cleanup the parent and join the memory thread if running."""
        super().cleanup()

        if self.memory_thread is not None:
            self.memory_thread.join()

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
