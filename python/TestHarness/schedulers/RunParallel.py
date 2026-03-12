# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import traceback
import platform
import os
from typing import TYPE_CHECKING
from importlib.util import find_spec
from time import perf_counter
from typing import Optional

from TestHarness.schedulers.Scheduler import Scheduler
from TestHarness import util
from TestHarness.runners.SubprocessRunner import Runner, SubprocessRunner

if platform.system() != "Windows" and find_spec("psutil") is not None or TYPE_CHECKING:
    from TestHarness.utils.monitor_processes import MemoryMonitor


class RunParallel(Scheduler):
    """
    RunParallel is a Scheduler plugin responsible for executing a tester
    command and doing something with its output.
    """

    @staticmethod
    def validParams():
        params = Scheduler.validParams()
        return params

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self._memory_monitor: Optional[MemoryMonitor] = None
        """The MemoryMonitor for sampling child process memory, if any."""

        if self.scheduler_options.monitor_job_memory:
            self._memory_monitor = MemoryMonitor(os.getpid(), interval=0.1)

    def run(self, job):
        """Run a tester command"""
        # Build and set the runner that will actually run the commands
        # This is abstracted away so we can support local runners and PBS/slurm runners
        job.setRunner(self.buildRunner(job, self.options))

        tester = job.getTester()

        # Do not execute app, and do not processResults
        if self.options.dry_run:
            self.setSuccessfulMessage(tester)
            return
        # Load results from a previous run
        elif self.options.show_last_run:
            job.loadPreviousResults()
            return

        # Start job timer
        job.timer.startMain()

        # Anything that throws while running or processing a job should be caught
        # and the job should fail
        try:
            # Launch and wait for the command to finish
            job.run()

            # Set the successful message
            if not tester.isSkip() and not job.isFail():
                self.setSuccessfulMessage(tester)
        except:
            trace = traceback.format_exc()
            job.appendOutput(
                util.outputHeader("Python exception encountered in Job") + trace
            )
            job.setStatus(job.error, "JOB EXCEPTION")
        finally:
            # Stop job timer
            job.timer.stopMain()

    def buildRunner(self, job, options) -> Runner:
        """Builds the runner for a given tester

        This exists as a method so that derived schedulers can change how they
        run commands (i.e., for PBS and slurm)
        """
        return SubprocessRunner(job, options, self.scheduler_options)

    def setSuccessfulMessage(self, tester):
        """properly set a finished successful message for tester"""
        message = ""

        # Handle 'dry run' first, because if true, job.run() never took place
        if self.options.dry_run:
            message = "DRY RUN"

        elif tester.specs["check_input"]:
            message = "SYNTAX PASS"

        elif self.options.scaling and tester.specs["scale_refine"]:
            message = "SCALED"

        elif (
            self.options.enable_recover
            and tester.specs.isValid("skip_checks")
            and tester.specs["skip_checks"]
        ):
            message = "PART1"

        tester.setStatus(tester.success, message)

    def waitFinish(self):
        # Start the memory monitor if enabled
        if self.scheduler_options.monitor_job_memory:
            self._memory_monitor = MemoryMonitor(os.getpid(), interval=0.1)
            self._memory_monitor.start()

        super().waitFinish()

    def monitorJobProcesses(self):
        """Monitor the running job processes, if enabled."""
        # Process memory monitoring is disabled
        if self._memory_monitor is None:
            return

        # Get the PIDs of the current running jobs so that they can be sampled
        pid_to_job = self.getActiveJobPIDMap()

        # Get latest samples from the monitor
        samples = self._memory_monitor.get_samples()

        # Update job memory and kill jobs over memory
        max_memory_per_slot = self.options.max_memory_per_slot
        for pid, job in pid_to_job.items():
            if (memory := samples.get(pid)) is not None:
                # If the job is already an error (we killed it), nothing to do:
                if job.getStatus() == job.error:
                    continue

                # Update max memory, if greater; this is True if it was greater
                updated = job.getRunner().updateMaxMemory(memory)

                # Check memory usage if needed
                if updated and max_memory_per_slot:
                    memory_per_slot_mb = memory / job.getSlots() * 2**-20
                    if memory_per_slot_mb > max_memory_per_slot:
                        message = (
                            "JOB KILLED (OVER MEMORY): "
                            f"Memory/slot {memory_per_slot_mb:.2f} "
                            f"MB > allowed {max_memory_per_slot:.2f} MB"
                        )
                        job.killProcess(job.error, "KILLED: OVER MEMORY", message)
