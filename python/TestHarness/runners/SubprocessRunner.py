# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os, importlib, platform, subprocess, shlex, sys, time
from contextlib import suppress
from tempfile import SpooledTemporaryFile
from threading import Thread
from typing import Optional, TYPE_CHECKING
from signal import SIGTERM
from TestHarness.runners.Runner import Runner
from TestHarness import util

# Try to load psutil; not a strict requirement but
# enables tracking memory
if TYPE_CHECKING:
    import psutil
else:
    if importlib.util.find_spec("psutil") is not None:
        import psutil
    else:
        psutil = None


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
        # The memory checking thread, created during spawn()
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
        self.outfile = SpooledTemporaryFile(max_size=1000000)  # 1M character buffer
        self.errfile = SpooledTemporaryFile(max_size=100000)  # 100K character buffer

        process_args = [cmd]
        process_kwargs = {
            "stdout": self.outfile,
            "stderr": self.errfile,
            "close_fds": False,
            "shell": use_shell,
            "cwd": tester.getTestDir(),
        }
        # On Windows, there is an issue with path translation when the command is passed in
        # as a list.
        if platform.system() == "Windows":
            process_kwargs["creationflags"] = subprocess.CREATE_NEW_PROCESS_GROUP
        else:
            process_kwargs["preexec_fn"] = os.setsid

        # Augment the environment if needed
        process_env = tester.augmentEnvironment(self.options)

        # Special logic for openmpi runs
        if tester.hasOpenMPI():
            # Don't clobber state
            process_env["OMPI_MCA_orte_tmpdir_base"] = self.job.getTempDirectory().name
            # Allow oversubscription for hosts that don't have a hostfile
            process_env["PRTE_MCA_rmaps_default_mapping_policy"] = ":oversubscribe"

        # Add to environment if requested
        if process_env:
            tester.setEnvironmentRan(process_env)
            process_kwargs["env"] = os.environ.copy()
            process_kwargs["env"].update(process_env)

        try:
            self.process = subprocess.Popen(*process_args, **process_kwargs)
        except Exception as e:
            raise Exception("Error in launching a new task") from e

        timer.start("runner_run")

        # Setup the memory checking thread if psutil is available;
        # disabled until it is no longer a performance hit on
        # python 3.14 (see #32243)
        # if psutil is not None:
        #     self.memory_thread = Thread(target=self._runMemoryThread)
        #     self.memory_thread.start()

    @staticmethod
    def getProcessMemory(process) -> Optional[int]:
        """Get an approximation for a process' total memory in bytes if possible."""
        assert psutil is not None
        assert isinstance(process, psutil.Process)

        # Use proportional set size (PSS) for linux, which is
        # a better approximation for MPI processes
        if sys.platform.startswith("linux"):
            with (
                suppress(FileNotFoundError),
                suppress(ProcessLookupError),
                open(f"/proc/{process.pid}/smaps_rollup", "r") as f,
            ):
                for line in f:
                    if line.startswith("Pss:"):  # in kB
                        return int(line.split()[1]) * 1000
        # Otherwise, use RSS (will double count shared memory)
        else:
            with suppress(psutil.Error):
                return process.memory_info().rss

        return None

    def _runMemoryThread(self):
        """Run the thread that tracks memory usage."""
        # Set to zero so that we can show that it is possible
        # to track memory even if the process is too short
        self.setMaxMemory(0)

        assert psutil is not None
        process = self.process
        assert process is not None

        # Capture the psutul.Process around the main process
        # so that we can recursively get all children pids
        # for getting their memory usage too
        try:
            psutil_process = psutil.Process(process.pid)
        except psutil.NoSuchProcess:
            return

        # See if we can track memory by checking with the main
        # process once; if we can't, there's nothing to do here
        # or the process has already finished
        if self.getProcessMemory(psutil_process) is None:
            return

        # Get the --max-memory option if set, and if so,
        # the number of procs (so that we can scale
        # memory used to per process)
        max_memory = self.options.max_memory
        procs = self.job.getTester().getProcs(self.options) if max_memory else None

        # Wait for the process to finish, checking memory per step
        max_found = 0
        while process.poll() is None:
            # Collect the parent + children processes so that
            # we can inspect memory for each
            psutil_processes = [psutil_process]
            with suppress(psutil.NoSuchProcess):
                psutil_processes += psutil_process.children(recursive=True)

            # Accumulate total across all ranks
            all_memory = [self.getProcessMemory(p) for p in psutil_processes]
            total = sum(v for v in all_memory if v is not None)

            # If have a zero value and the main process isn't running,
            # we're done here
            if total == 0 and not process.poll():
                return

            # If memory has increased, set it so that it can be
            # reported live during a long-running job
            if total > max_found:
                self.setMaxMemory(total)
                max_found = total

                # If --max-memory is set, make sure we're not over
                if max_memory is not None:
                    assert isinstance(procs, int)
                    rss_per_proc = float(max_found / procs) * 1.0e-6

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

        self.process.wait()

        timer.stop("runner_run")

        self.exit_code = self.process.poll()

        # This should have been cleared before the job started
        if self.getRunOutput().hasOutput():
            raise Exception("Runner run output was not cleared")

        # Load combined output
        for file in [self.outfile, self.errfile]:
            file.flush()
            output = self.readOutput(file)
            file.close()

            # For some reason openmpi will append a null character at the end
            # when the exit code is nonzero. Not sure why this is... but remove
            # it until we figure out what's broken
            if (
                file == self.errfile
                and self.exit_code != 0
                and self.job.getTester().hasOpenMPI()
                and len(output) > 2
                and output[-3:] in ["\n\0\n", "\n\x00\n"]
            ):
                output = output[:-3]

            self.getRunOutput().appendOutput(output)

    def kill(self):
        if self.process is not None:
            try:
                if platform.system() == "Windows":
                    from distutils import spawn

                    if spawn.find_executable("taskkill"):
                        subprocess.call(
                            ["taskkill", "/F", "/T", "/PID", str(self.process.pid)]
                        )
                    else:
                        self.process.terminate()
                else:
                    pgid = os.getpgid(self.process.pid)
                    os.killpg(pgid, SIGTERM)
            except OSError:  # Process already terminated
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
