# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import platform
import re
import shlex
import subprocess
import time
from signal import SIGTERM
from tempfile import SpooledTemporaryFile
from threading import Lock
from typing import Optional

from TestHarness.mpi_config import MPIType
from TestHarness.runners.Runner import Runner
from TestHarness.schedulers.Scheduler import SchedulerOptions, TimeUtilityType


class SubprocessRunner(Runner):
    """Runner that spawns a local subprocess."""

    def __init__(self, job, options, scheduler_options: SchedulerOptions):
        Runner.__init__(self, job, options, scheduler_options)

        self.outfile: Optional[SpooledTemporaryFile] = None
        """The output file handler."""
        self.errfile: Optional[SpooledTemporaryFile] = None
        """The error file handler."""

        self.process: Optional[subprocess.Popen] = None
        """The underlying process."""
        self._pid: Optional[int] = None
        """The running process' process ID, if any."""
        self._pid_lock = Lock()
        """The lock for self._pid."""

        self.monitor_time = False
        """Whether or not we're monitoring the process with a time utility."""

    @property
    def pid(self) -> Optional[int]:
        """Get the pid of the running process, if any."""
        with self._pid_lock:
            return self._pid

    def timeFilePath(self):
        """Get the path to the time file."""
        return f"{self.job.getOutputPathPrefix()}.testharness_time"

    def spawn(self, timer):
        tester = self.job.getTester()
        use_shell = tester.specs["use_shell"]
        cmd = tester.getCommand(self.options)

        # If the tester supports wrapping with a time utility
        # and we're not using shell, wrap the command with
        # a time utility if a timing utility is available
        if (
            self.scheduler_options.monitor_job_cpu
            and tester.SUPPORTS_TIME
            and not use_shell
        ):
            time_utility_type = self.scheduler_options.time_utility_type
            assert time_utility_type != TimeUtilityType.NONE

            time_utility_path = self.scheduler_options.time_utility_path
            assert time_utility_path is not None

            time_command = f"{time_utility_path} -o {self.timeFilePath()}"

            # GNU time; enable quiet and just output CPU %
            if self.scheduler_options.time_utility_type == TimeUtilityType.GNU:
                time_command += " -f '%P' -q"

            cmd = f"{time_command} {cmd}"
            self.monitor_time = True
            self.deleteTimeFile(graceful=True)
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

        # OpenMPI, allow for oversubscription
        if (
            not self.options.disable_openmpi_oversubscribe
            and self.scheduler_options.mpi_config.mpi_type == MPIType.OPENMPI
        ):
            # Don't clobber state
            process_env["OMPI_MCA_orte_tmpdir_base"] = self.job.getTempDirectory().name
            # Allow oversubscription for hosts that don't have a hostfile
            process_env["PRTE_MCA_rmaps_default_mapping_policy"] = ":oversubscribe"
        # hwloc with a prebuilt topology file
        if hwloc_topo_file := self.scheduler_options.hwloc_topology_path:
            process_env["HWLOC_XMLFILE"] = hwloc_topo_file
            process_env["HWLOC_THISSYSTEM"] = "1"

        # Add to environment if requested
        if process_env:
            tester.setEnvironmentRan(process_env)
            process_kwargs["env"] = os.environ.copy()
            process_kwargs["env"].update(process_env)

        try:
            self.process = subprocess.Popen(*process_args, **process_kwargs)
        except Exception as e:
            raise Exception("Error in launching a new task") from e

        with self._pid_lock:
            self._pid = self.process.pid
        timer.start("runner_run")

    def wait(self, timer):
        assert self.process is not None

        self.process.wait()

        timer.stop("runner_run")

        self.exit_code = self.process.poll()

        # Make the pid no longer available
        with self._pid_lock:
            self._pid = None

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
                and self.scheduler_options.mpi_config.mpi_type == MPIType.OPENMPI
                and len(output) > 2
                and output[-3:] in ["\n\0\n", "\n\x00\n"]
            ):
                output = output[:-3]

            self.getRunOutput().appendOutput(output)

        # Parse the time output if we have one
        if self.monitor_time:
            self.parseTimeFile(timer)

    def parseTimeFile(self, timer):
        """Parse the time file."""
        assert self.monitor_time

        # Load the time utility output
        try:
            with open(self.timeFilePath(), "r") as f:
                contents = f.read().strip()
        # File didn't exist
        except FileNotFoundError:
            self.job.setStatus(self.job.error, "TIME FILE MISSING")
            self.appendOutput(f"\n\nFailed to find time file {self.timeFilePath()}")
        # Obtain the CPU percentage from the time output
        else:
            time_type = self.scheduler_options.time_utility_type

            # GNU time; should contain just the percentage
            if time_type == TimeUtilityType.GNU:
                if match := re.fullmatch(r"(\d+)%", contents):
                    self.cpu_percent = float(match.group(1))
            # BSD time; need to compute percentage from (user+sys)/real
            elif time_type == TimeUtilityType.BSD and (
                match := re.fullmatch(
                    r"(\d+.\d+) real \s+ (\d+.\d+) user \s+ (\d+.\d+) sys", contents
                )
            ):
                real_time = float(match.group(1))
                if real_time > 0:
                    user_time = float(match.group(2))
                    sys_time = float(match.group(3))
                    self.cpu_percent = (user_time + sys_time) / real_time * 100.0
                else:
                    self.cpu_percent = 0.0

            # Didn't find it or the parse failed; error if we haven't
            # already hit an error or a timeout
            if self.cpu_percent is None and self.job.getStatus() not in [
                self.job.error,
                self.job.timeout,
            ]:
                self.job.setStatus(self.job.error, "TIME FILE FAILURE")
                self.appendOutput(
                    (
                        f"\n\nFailed to parse time file {self.timeFilePath()}; "
                        f"contents:\n\n{contents}"
                    )
                )

    def deleteTimeFile(self, graceful: bool):
        """Delete the time file."""
        assert self.monitor_time
        try:
            os.remove(self.timeFilePath())
        except FileNotFoundError:
            if not graceful:
                raise

    def cleanup(self):
        super().cleanup()

        # Cleanup the time file if it exists
        if self.monitor_time:
            self.deleteTimeFile(graceful=True)

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
