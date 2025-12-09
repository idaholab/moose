#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, platform, subprocess, shlex, time
from importlib.util import find_spec
from contextlib import suppress
from tempfile import SpooledTemporaryFile
from threading import Thread
from typing import Optional
from signal import SIGTERM
from TestHarness.runners.Runner import Runner
from TestHarness import util

process_stats = None
with suppress(ImportError):
    from TestHarness.utils.process_stats import process_stats

HAS_PSUTIL = find_spec("psutil") is not None
"""Whether or not psutil is available; needed for the stats thread."""
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
        # The stats thread, created during spawn()
        self.stats_thread: Optional[Thread] = None

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

        # Augment the environment if needed
        process_env = tester.augmentEnvironment(self.options)

        # Special logic for openmpi runs
        if tester.hasOpenMPI():
            # Don't clobber state
            process_env['OMPI_MCA_orte_tmpdir_base'] = self.job.getTempDirectory().name
            # Allow oversubscription for hosts that don't have a hostfile
            process_env['PRTE_MCA_rmaps_default_mapping_policy'] = ':oversubscribe'

        # Add to environment if requested
        if process_env:
            process_kwargs["env"] = os.environ.copy()
            process_kwargs["env"].update(process_env)

        try:
            self.process = subprocess.Popen(*process_args, **process_kwargs)
        except Exception as e:
            raise Exception('Error in launching a new task') from e

        timer.start('runner_run')

        # Setup the memory checking thread if process_stats is available
        if process_stats is not None:
            self.stats_thread = Thread(target=self._runStatThread)
            self.stats_thread.start()

    def _runStatThread(self):
        """Run the thread that tracks memory usage."""
        assert process_stats is not None
        process = self.process
        assert process is not None

        # Set to zero so that we can show that it is possible
        # to track memory even if the process is too short
        self.setMaxMemory(0)

        # Max CPU percent allowed; scale by slots and allow a
        # 25% buffer for each slot
        max_cpu_percent = 100.0 * self.job.getSlots() * 1.25

        # Max memory allowed in bytes, if a memory max is set
        # in the options; if so, scale by number of processors
        max_memory_option = self.options.max_memory
        allowed_max_memory = None
        if max_memory_option:
            procs = self.job.getTester().getProcs(self.options)
            allowed_max_memory = int(max_memory_option * procs * 1e6)

        # Helper for killing the underlying process
        def kill(message, status):
            self.kill()
            self.appendOutput("\n" + util.outputHeader(message, False))
            self.job.setStatus(self.job.error, status)

        current_max_memory = 0

        # Work through the updates
        for stats in process_stats(pid=process.pid, poll_time=0.05, update_interval=2):
            # Check max CPU
            if stats.cpu_percent > max_cpu_percent:
                kill(
                    (
                        f"Job killed: CPU {stats.cpu_percent:.2f}% "
                        f"> allowed {max_cpu_percent:.2f}%"
                    ),
                    "EXCEEDED CPU",
                )

            # Max memory increased
            if stats.max_memory > current_max_memory:
                # Update so that it shows up in tester status
                self.setMaxMemory(stats.max_memory)
                current_max_memory = stats.max_memory

                # Check max memory if limited
                if allowed_max_memory and stats.max_memory > allowed_max_memory:
                    kill(
                        (
                            f"Job killed: memory/slot {int(stats.max_memory / 1e6)}MB "
                            f"> max {int(allowed_max_memory / 1e6)}MB"
                        ),
                        "EXCEEDED MEMORY",
                    )

    def wait(self, timer):
        assert self.process is not None

        self.process.wait()

        timer.stop('runner_run')

        self.exit_code = self.process.poll()

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

        # Wait for the stats thread to finish up
        if self.stats_thread is not None:
            self.stats_thread.join()

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
