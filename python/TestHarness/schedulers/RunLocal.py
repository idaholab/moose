# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
from typing import TYPE_CHECKING, Optional

from TestHarness.runners.SubprocessRunner import Runner, SubprocessRunner
from TestHarness.schedulers.Scheduler import Scheduler

CANNOT_MONITOR_JOB_MEMORY_REASON: Optional[str] = None
"""Reason for not being able to monitor job memory, if any."""
try:
    import TestHarness.utils.monitor_processes
except ImportError as e:
    CANNOT_MONITOR_JOB_MEMORY_REASON = str(e)

if CANNOT_MONITOR_JOB_MEMORY_REASON is None or TYPE_CHECKING:
    from TestHarness.utils.monitor_processes import (
        NO_CUDA_TRACKING_REASON,
        MemoryMonitor,
    )


class RunLocal(Scheduler):
    """A scheduler for executing tester commands locally."""

    CAN_SET_HWLOC_TOPOLOGY = True
    CAN_SET_MAX_CPU_MEMORY = True
    CAN_SET_MAX_GPU_MEMORY = True
    CAN_OPENMPI_OVERSUBSCRIBE = True
    MONITOR_JOB_CPU = True
    MONITOR_JOB_MEMORY = CANNOT_MONITOR_JOB_MEMORY_REASON is None

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self._memory_monitor: Optional[MemoryMonitor] = None
        """The MemoryMonitor for sampling child process memory, if any."""

        self._track_cpu_memory: bool = False
        """Whether or not CPU memory is being tracked."""

        self._track_gpu_memory: bool = False
        """Whether or not CPU memory is being tracked."""

        # Setup the monitor if enabled
        track_nvidia = False
        if self.scheduler_options.monitor_job_memory:
            # Cannot monitor Job memory for some reason
            if CANNOT_MONITOR_JOB_MEMORY_REASON is not None:
                self.harness.errorExit(
                    "Cannot monitor job memory: "
                    + CANNOT_MONITOR_JOB_MEMORY_REASON
                    + "; set --no-memory-tracking"
                )

            # Whether or not we should track Nvidia GPU memory
            track_nvidia = False
            if self.scheduler_options.monitor_job_gpu_memory:
                track_nvidia = self.options.compute_device == "cuda"
                if track_nvidia and NO_CUDA_TRACKING_REASON is not None:
                    self.harness.errorExit(
                        "Cannot monitor job GPU memory: "
                        + NO_CUDA_TRACKING_REASON
                        + "; set --no-gpu-memory-tracking"
                    )

            self._memory_monitor = MemoryMonitor(
                os.getpid(),
                track_nvidia=track_nvidia,
                interval=self.scheduler_options.memory_tracking_interval,
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
        max_cpu_memory_per_slot = self.options.max_cpu_memory_per_slot
        max_gpu_memory_per_slot = self.options.max_gpu_memory_per_slot
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

            # Update max memory; this will return True if something was updated
            # (we hit a higher max). Thus, if not updated, return as we have
            # no checking to perform
            if not runner.updateMaxMemory(job_cpu_memory_bytes, job_gpu_memory_bytes):
                return

            # Utility for checking CPU and GPU memory if appropriate
            def check_memory(
                memory_per_slot: float, memory_bytes: int, memory_type: str
            ):
                if (
                    memory_per_slot
                    and (
                        job_memory_per_slot_mb := (
                            memory_bytes / job.getSlots() * 2**-20
                        )
                    )
                    > memory_per_slot
                ):
                    message = (
                        f"JOB KILLED (OVER {memory_type} MEMORY): "
                        f"{memory_type} Memory/slot {job_memory_per_slot_mb:.2f} "
                        f"MB > allowed {memory_per_slot:.2f} MB"
                    )
                    job.killProcess(
                        job.error, f"KILLED: OVER {memory_type} MEMORY", message
                    )

            check_memory(max_cpu_memory_per_slot, job_cpu_memory_bytes, "CPU")
            if job_gpu_memory_bytes:
                check_memory(max_gpu_memory_per_slot, job_gpu_memory_bytes, "GPU")

    def tracksCPUMemory(self) -> bool:
        """Whether or not CPU memory is being tracked."""
        return self._track_cpu_memory

    def tracksGPUMemory(self) -> bool:
        """Whether or not GPU memory is being tracked."""
        return self._track_gpu_memory
