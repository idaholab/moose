#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import asyncio, os, platform, subprocess, shlex, time
from importlib.util import find_spec
from contextlib import suppress
from queue import Queue
from tempfile import SpooledTemporaryFile
from threading import Thread
from typing import Optional, TYPE_CHECKING
from signal import SIGTERM
from TestHarness.runners.Runner import Runner
from TestHarness import util

if TYPE_CHECKING:
    from TestHarness.utils.monitor_process import KilledReason, monitor_process
else:
    monitor_process = None
    # monitor_process requires psutil
    if find_spec("psutil") is not None:
        from TestHarness.utils.monitor_process import KilledReason, monitor_process

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
        # Max percent CPU allowed, if any
        self.allowed_percent_cpu: Optional[float] = None
        # Max memory allowed, in bytes, if any
        self.allowed_memory: Optional[int] = None
        # The monitor thread, created during spawn()
        self.monitor_thread: Optional[Thread] = None
        # The queue for the monitor thread
        self.monitor_queue: Optional[Queue] = None

    def spawn(self, timer):
        tester = self.job.getTester()
        use_shell = tester.specs["use_shell"]
        cmd = tester.getCommand(self.options)
        tester.setCommandRan(cmd)

        # Split command into list of args to be passed to Popen
        if not use_shell:
            cmd = shlex.split(cmd)

        # Max CPU percent allowed; allow an extra 25% for the first
        # slot and an extra 10% for the rest
        self.allowed_percent_cpu = 100.0 * (1.25 + ((self.job.getSlots() - 1) * 1.1))

        # Max memory allowed in bytes, if a memory max is set
        # in the options; if so, scale by number of processors
        max_memory_option = self.options.max_memory
        if max_memory_option:
            procs = self.job.getTester().getProcs(self.options)
            self.allowed_memory = int(max_memory_option * procs * 1e6)

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
            tester.setEnvironmentRan(process_env)
            process_kwargs["env"] = os.environ.copy()
            process_kwargs["env"].update(process_env)

        try:
            self.process = subprocess.Popen(*process_args, **process_kwargs)
        except Exception as e:
            raise Exception('Error in launching a new task') from e

        timer.start('runner_run')

        # Setup the monitor thread if available
        if monitor_process is not None:
            self.monitor_queue = Queue(maxsize=1)
            self.monitor_thread = Thread(
                target=monitor_process,
                kwargs={
                    'pid': self.process.pid,
                    'poll_time': 0.025,
                    'cpu_update_interval': 20,
                    'memory_update_interval': 10,
                    'result_queue': self.monitor_queue,
                    'max_percent_cpu': self.allowed_percent_cpu,
                    'max_memory': self.allowed_memory
                },
                daemon=True
            )
            self.monitor_thread.start()

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

        # Wait for the monitor thread to finish up
        if self.monitor_thread is not None:
            assert self.monitor_queue is not None

            self.monitor_thread.join()
            result = self.monitor_queue.get()

            if result.max_memory:
                self.setMaxMemory(result.max_memory)

            if result.killed is not None:
                def fail(reason, status):
                    message = f"Job killed: {reason}; process info:"
                    self.appendOutput("\n" + util.outputHeader(message, False))
                    self.appendOutput("\n" + result.killed_info)
                    self.job.setStatus(self.job.error, status)

                if result.killed == KilledReason.CPU:
                    fail(
                        (
                            f"CPU {result.max_percent_cpu:.2f}% "
                            f"> allowed {self.allowed_percent_cpu:.2f}%"
                        ),
                        "EXCEEDED CPU",
                    )
                else:
                    assert self.allowed_memory is not None
                    fail(
                        (
                            f"memory/slot {int(result.max_memory / 1e6)}MB "
                            f"> max {int(self.allowed_memory / 1e6)}MB"
                        ),
                        "EXCEEDED MEMORY",
                    )

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
