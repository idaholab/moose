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
from typing import Optional, Tuple
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

    def _waitProcess(self):
        """
        Wait for a process to complete.

        Sets the max memory while running and the exit code on completion.
        """
        assert self.process is not None
        psutil_process = psutil.Process(self.process.pid)

        max_rss = 0
        self.setMaxMemory(0)

        while self.process.poll() is None:
            psutil_processes = []
            with suppress(psutil.NoSuchProcess):
                psutil_processes = [psutil_process] + psutil_process.children(
                    recursive=True
                )

            total = 0
            for p in psutil_processes:
                with suppress(psutil.NoSuchProcess):
                    total += p.memory_info().rss

            if total > max_rss:
                self.setMaxMemory(total)
                max_rss = total

            time.sleep(0.01)

        self.process.wait()
        self.exit_code = self.process.poll()

    def wait(self, timer):
        self._waitProcess()
        assert self.exit_code is not None
        assert self.max_memory is not None

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
