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
from importlib.util import find_spec
from typing import TYPE_CHECKING, Optional

from TestHarness.runners.SubprocessRunner import Runner, SubprocessRunner
from TestHarness.schedulers.Scheduler import Scheduler

_MONITOR_JOB_MEMORY = platform.system() != "Windows" and find_spec("psutil") is not None
if _MONITOR_JOB_MEMORY or TYPE_CHECKING:
    from TestHarness.utils.monitor_processes import MemoryMonitor


class RunLocal(Scheduler):
    """A scheduler for executing tester commands locally."""

    CAN_SET_HWLOC_TOPOLOGY = True
    CAN_SET_MAX_MEMORY = True
    CAN_OPENMPI_OVERSUBSCRIBE = True
    MONITOR_JOB_CPU = True
    MONITOR_JOB_MEMORY = _MONITOR_JOB_MEMORY

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self._memory_monitor: Optional[MemoryMonitor] = None
        """The MemoryMonitor for sampling child process memory, if any."""

        self._track_cpu_memory: bool = False
        """Whether or not CPU memory is being tracked."""

        self._track_gpu_memory: bool = False
        """Whether or not CPU memory is being tracked."""

        # Setup the monitor if enabled
        if self.scheduler_options.monitor_job_memory:
            # Whether or not we should track Nvidia GPU memory
            track_nvidia = self.options.compute_device == "cuda"

            self._memory_monitor = MemoryMonitor(
                os.getpid(), track_nvidia=track_nvidia, interval=0.1
            )

            self._track_cpu_memory = True
            if track_nvidia:
                self._track_gpu_memory = True

    def buildRunner(self, job, options) -> Runner:
        """Build a SubprocessRunner."""
        return SubprocessRunner(job, options, self.scheduler_options)

    def waitFinish(self):
        # Start the memory monitor if enabled
        if self._memory_monitor is not None:
            self._memory_monitor.start()

        super().waitFinish()

        if self._memory_monitor:
            self._memory_monitor.stop()

    def monitorJobProcesses(self):
        """Monitor the running job processes, if enabled."""
        # Process memory monitoring is disabled
        if self._memory_monitor is None:
            return

        # Get the PIDs of the current running jobs so that they can be sampled
        pid_to_job = self.getActiveJobPIDMap()

        # Get latest samples from the monitor
        cpu_samples = self._memory_monitor.get_cpu_samples()
        gpu_samples = self._memory_monitor.get_gpu_samples()

        # Update job memory and kill jobs over memory
        max_memory_per_slot_mb = self.options.max_memory_per_slot
        for pid, job in pid_to_job.items():
            runner = job.getRunner()
            assert runner is not None

            job_cpu_memory_bytes = cpu_samples.get(pid)

            # Didn't get samples for this Job's process
            if job_cpu_memory_bytes is None:
                continue

            # If the job is already an error (we may have killed it), nothing to do
            if job.getStatus() == job.error:
                continue

            job_gpu_memory_bytes = (
                gpu_samples.get(pid, 0) if gpu_samples is not None else None
            )

            # Update max memory, if greater; this is True if greater
            updated = runner.updateMaxMemory(job_cpu_memory_bytes, job_gpu_memory_bytes)

            # Check memory usage if needed
            if (
                updated
                and max_memory_per_slot_mb
                and (
                    job_memory_per_slot_mb := (
                        job_cpu_memory_bytes / job.getSlots() * 2**-20
                    )
                )
                > max_memory_per_slot_mb
            ):
                message = (
                    "JOB KILLED (OVER MEMORY): "
                    f"Memory/slot {job_memory_per_slot_mb:.2f} "
                    f"MB > allowed {max_memory_per_slot_mb:.2f} MB"
                )
                job.killProcess(job.error, "KILLED: OVER MEMORY", message)

    def tracksCPUMemory(self) -> bool:
        """Whether or not CPU memory is being tracked."""
        return self._track_cpu_memory

    def tracksGPUMemory(self) -> bool:
        """Whether or not GPU memory is being tracked."""
        return self._track_gpu_memory
