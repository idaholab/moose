# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement monitor_process() for monitoring a process' cpu and memory usage."""

import asyncio
import os
import platform
import sys
from contextlib import suppress
from dataclasses import dataclass
from enum import Enum
from importlib.util import find_spec
from queue import Queue
from signal import SIGTERM
from time import perf_counter
from typing import TYPE_CHECKING, Optional

if TYPE_CHECKING:
    import psutil
else:
    if find_spec("psutil") is not None:
        import psutil
    else:
        raise ModuleNotFoundError("monitor_process() requires the 'psutil' package")

MEMORY_PSS = sys.platform.startswith("linux") and os.path.exists(
    f"/proc/{os.getpid()}/smaps_rollup"
)
"""Whether or not to use PSS for memory tracking from smaps_rollup."""


@dataclass
class ProcessSample:
    """Single sample for a process' cpu and memory usage."""

    pid: int
    """The process ID."""
    wall_time: float
    """Wall time at which the sample was made in seconds."""
    cpu_time: float
    """CPU time of the process in seconds during the sample."""
    memory: int
    """Memory usage of the process in bytes during the sample."""


async def poll_process(pid: int) -> list[ProcessSample]:
    """
    Sample a process and all of its children.

    This is a heaver call so it is asynchronous.
    """
    def _poll_process() -> list[ProcessSample]:
        samples: list[ProcessSample] = []

        # Process has ended
        try:
            process = psutil.Process(pid)
        except psutil.NoSuchProcess:
            return []

        # Capture process + all of its children
        all_processes = [process]
        with suppress(psutil.NoSuchProcess):
            all_processes += process.children(recursive=True)

        # Sample all processes
        for p in all_processes:
            # Sample CPU, or skip if it has ended
            try:
                times = p.cpu_times()
            except psutil.NoSuchProcess:
                continue
            sample = ProcessSample(
                pid=p.pid,
                wall_time=perf_counter(),
                cpu_time=(times.user + times.system),
                memory=0,
            )

            # Sample memory if available
            if MEMORY_PSS:
                with (
                    suppress(FileNotFoundError),
                    suppress(ProcessLookupError),
                    open(f"/proc/{process.pid}/smaps_rollup", "r") as f,
                ):
                    for line in f:
                        if line.startswith("Pss:"):  # in kB
                            sample.memory = int(line.split()[1]) * 1000
            # Otherwise, use RSS (will double count shared memory)
            else:
                with suppress(psutil.NoSuchProcess):
                    sample.memory = process.memory_info().rss

            samples.append(sample)

        return samples

    # Dispatch to another thread (this is a heavier call)
    return await asyncio.to_thread(_poll_process)


class KilledReason(Enum):
    """The reason why a monitored process was killed."""

    MEMORY = 1
    """Killed due to over memory."""
    CPU = 2
    """Killed due to over CPU."""


@dataclass
class MonitoredProcess:
    """Result for a process monitored with monitor_process()."""

    max_memory: int = 0
    """The estimated maximum memory in bytes."""
    max_percent_cpu: float = 0.0
    """The estimated maximum percent CPU."""
    killed: Optional[KilledReason] = None
    """The reason the process was killed, if any."""


async def _monitor_process(
    pid: int,
    poll_time: float,
    update_interval: int,
    max_percent_cpu: Optional[float],
    max_memory: Optional[int],
) -> MonitoredProcess:
    # The final result
    result = MonitoredProcess()
    # The samples from the previous iteration
    last_samples: dict[int, ProcessSample] = {}

    # Process has already ended, nothing to do
    try:
        process = psutil.Process(pid)
    except psutil.NoSuchProcess:
        return result

    # Helper for killing the process and setting the killed state
    def kill(reason: KilledReason):
        if platform.system() == "Windows":
            process.terminate()
        else:
            with suppress(OSError):
                os.killpg(os.getpgid(pid), SIGTERM)
        result.killed = reason
        return result

    num_intervals = 0
    while process.is_running():
        num_intervals += 1

        # Have reached an update point
        if num_intervals == update_interval:
            # Reset as we're updating
            num_intervals = 0

            # Get current samples
            samples = await poll_process(pid)

            # No samples, we're done
            if not samples:
                return result

            # Update max memory
            memory = sum(v.memory for v in samples)
            if memory > result.max_memory:
                result.max_memory = memory

            # Update max percent cpu
            percent_cpu = 0.0
            for sample in samples:
                if (last_sample := last_samples.get(sample.pid)) is not None:
                    percent_cpu += (
                        100.0
                        * (sample.cpu_time - last_sample.cpu_time)
                        / (sample.wall_time - last_sample.wall_time)
                    )
            if percent_cpu > result.max_percent_cpu:
                result.max_percent_cpu = percent_cpu

            # Check failure criteria
            if max_memory and memory > max_memory:
                return kill(KilledReason.MEMORY)
            if max_percent_cpu and result.max_percent_cpu > max_percent_cpu:
                return kill(KilledReason.CPU)

            # Copy current samples for next cpu calculation
            last_samples = {sample.pid: sample for sample in samples}

        # Poll
        await asyncio.sleep(poll_time)

    return result


def monitor_process(
    pid: int,
    poll_time: float,
    update_interval: int,
    result_queue: Queue[MonitoredProcess],
    max_percent_cpu: Optional[float] = None,
    max_memory: Optional[int] = None,
):
    """
    Monitor a process and all of its children for cpu and memory usage.

    The final result will be placed into the queue given by
    result_queue. The process polling is handled asynchronously
    as it is a heavier task.

    This should ideally be ran as a Thread, in which the queue should
    be read once the thread has been joined for the result.

    If max_percent_cpu or max_memory are set, the process will be killed
    and a killed state set in the queue.

    Parameters
    ----------
    pid : int
        The top-level ID of the process to monitor.
    poll_time : float
        How often to poll the process in seconds.
    update_interval : int
        How often to update process stats in number of polls.
    result_queue : Queue[MonitoredProcess]
        The queue to insert the final result into.

    Optional Parameters:
    -------------------
    max_percent_cpu : Optional[float]
        The max percent cpu allowed during a sample, if any.
    max_memory : Optional[int]
        The max memory allowed during a sample in bytes, if any.

    """
    async def runner():
        return await _monitor_process(
            pid=pid,
            poll_time=poll_time,
            update_interval=update_interval,
            max_percent_cpu=max_percent_cpu,
            max_memory=max_memory,
        )

    result_queue.put(asyncio.run(runner()))
