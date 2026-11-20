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
from TestHarness.util import printPrefixed

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
    MONITOR_JOB_GPU_MEMORY = _MONITOR_JOB_MEMORY

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self._memory_monitor: Optional[MemoryMonitor] = None
        """The MemoryMonitor for sampling child process memory, if any."""

        self._checked_gpu_error: bool = False
        """Whether or not the monitor has been checked for a GPU sampling failure."""

    def buildRunner(self, job, options) -> Runner:
        """Build a SubprocessRunner."""
        return SubprocessRunner(job, options, self.scheduler_options)

    def waitFinish(self):
        # Start the memory monitor if enabled
        if self.scheduler_options.monitor_job_memory:
            self._memory_monitor = MemoryMonitor(
                os.getpid(),
                interval=self.scheduler_options.memory_tracking_interval,
                track_gpu=self.scheduler_options.monitor_job_gpu_memory,
            )
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
        last_memory_samples = self._memory_monitor.get_samples()

        # Report once if GPU memory sampling failed to start, which the sampler
        # records before it publishes the first samples
        if (
            last_memory_samples
            and not self._checked_gpu_error
            and self.scheduler_options.monitor_job_gpu_memory
        ):
            self._checked_gpu_error = True
            if (gpu_error := self._memory_monitor.get_gpu_error()) is not None:
                printPrefixed(
                    "WARNING",
                    f"GPU memory is not tracked; sampling failed to start: {gpu_error}",
                    color="YELLOW" if self.options.colored else None,
                )

        # Update job memory and kill jobs over memory
        max_memory_per_slot_mb = self.options.max_memory_per_slot
        max_gpu_memory_per_slot_mb = self.options.max_gpu_memory_per_slot
        for pid, job in pid_to_job.items():
            sample = last_memory_samples.get(pid)

            # Didn't get samples for this Job's process
            if sample is None:
                continue

            # If the job is already an error (we may have killed it), nothing to do
            if job.getStatus() == job.error:
                continue

            # Update max memory, if greater; each is True if greater
            updated, gpu_updated = job.getRunner().updateMaxMemory(
                sample.cpu, sample.gpu
            )

            # Check memory usage if needed
            for name, prefix, check, memory_bytes, max_per_slot_mb in [
                ("MEMORY", "Memory", updated, sample.cpu, max_memory_per_slot_mb),
                (
                    "GPU MEMORY",
                    "GPU memory",
                    gpu_updated,
                    sample.gpu,
                    max_gpu_memory_per_slot_mb,
                ),
            ]:
                if (
                    check
                    and max_per_slot_mb
                    and (per_slot_mb := memory_bytes / job.getSlots() * 2**-20)
                    > max_per_slot_mb
                ):
                    message = (
                        f"JOB KILLED (OVER {name}): "
                        f"{prefix}/slot {per_slot_mb:.2f} "
                        f"MB > allowed {max_per_slot_mb:.2f} MB"
                    )
                    job.killProcess(job.error, f"KILLED: OVER {name}", message)
                    break
