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
from time import perf_counter, time
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
    wall_time: Optional[float] = None
    """Wall time at which the cpu sample was made in seconds, if sampled."""
    cpu_time: Optional[float] = None
    """Total CPU time of the process in seconds, if sampled."""
    memory: Optional[int] = None
    """Memory usage of the process in bytes, if samples."""


async def poll_process(pid: int, cpu: bool, memory: bool) -> list[ProcessSample]:
    """
    Sample a process and all of its children.

    This is a heaver call so it is asynchronous.
    """
    assert cpu or memory, "No sample options set"

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
            if not p.is_running():
                continue

            sample = None

            # Sample CPU if requested, or skip if ended
            if cpu:
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

            # Sample memory if requested
            if memory:
                p_memory = None

                # Prefer PSS if available
                if MEMORY_PSS:
                    with (
                        suppress(FileNotFoundError),
                        suppress(ProcessLookupError),
                        open(f"/proc/{p.pid}/smaps_rollup", "r") as f,
                    ):
                        for line in f:
                            if line.startswith("Pss:"):  # in kB
                                p_memory = int(line.split()[1]) * 1000
                                break
                # Otherwise, use RSS (will double count shared memory)
                else:
                    with suppress(psutil.NoSuchProcess):
                        p_memory = p.memory_info().rss

                if p_memory is not None:
                    if sample is None:
                        sample = ProcessSample(pid=p.pid, memory=p_memory)
                    else:
                        sample.memory = p_memory

            if sample is not None:
                samples.append(sample)

        return samples

    # Dispatch to another thread (this is a heavier call)
    try:
        return await asyncio.to_thread(_poll_process)
    except RuntimeError as e:
        # During interpreter shutdown, ThreadPoolExecutor forbids new tasks.
        # Treat that as "we're done monitoring" and return no samples.
        if "interpreter shutdown" in str(e) or "Event loop is closed" in str(e):
            return []
        raise
    except asyncio.CancelledError:
        return []


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
    killed_info: Optional[str] = None
    """Info about the running processes if killed."""


async def _monitor_process(
    pid: int,
    poll_time: float,
    cpu_update_interval: int,
    memory_update_interval: int,
    max_percent_cpu: Optional[float],
    max_memory: Optional[int],
) -> MonitoredProcess:
    # The final result
    result = MonitoredProcess()
    # The samples from the previous CPU update
    last_cpu_samples: dict[int, ProcessSample] = {}

    # Process has already ended, nothing to do
    try:
        process = psutil.Process(pid)
    except psutil.NoSuchProcess:
        return result

    # Get the percent cpu for a given sample, if available.
    # It is available if we have a previous sample for
    # the same process, as a delta is required.
    def sample_percent_cpu(sample: ProcessSample) -> float:
        if (last_sample := last_cpu_samples.get(sample.pid)) is not None:
            assert sample.cpu_time is not None
            assert last_sample.cpu_time is not None
            assert sample.wall_time is not None
            assert last_sample.wall_time is not None
            return (
                100.0
                * (sample.cpu_time - last_sample.cpu_time)
                / (sample.wall_time - last_sample.wall_time)
            )
        return 0.0

    # Helper for killing the process and setting the killed state
    def kill(reason: KilledReason, samples: list[ProcessSample]):
        # Accumulate as much info about the running processes
        # as we can to provide more context
        info = []
        for sample in samples:
            process_info = []
            with suppress(psutil.NoSuchProcess):
                p = psutil.Process(sample.pid)
                cmdline = " ".join(p.cmdline())
                parent = p.ppid()
                wall_time = time() - p.create_time()
                process_info.append(("Command", cmdline))
                process_info.append(("Parent", parent))
                process_info.append(("Wall time", f"{wall_time:.2f} sec"))
            if sample.memory is not None:
                process_info.append(("Memory", f"{(sample.memory / 1e6):.2f} MB"))
            if cpu := sample_percent_cpu(sample):
                process_info.append(("CPU", f"{cpu:.2f}%"))
            info.append(f"Process {sample.pid}:")
            info += [f"  {k}: {v}" for k, v in process_info]

        # Kill the process
        if platform.system() == "Windows":
            process.terminate()
        else:
            with suppress(OSError):
                os.killpg(os.getpgid(pid), SIGTERM)

        result.killed = reason
        result.killed_info = "\n".join(info)

        return result

    # Poll; every poll will check if the main process is
    # running. We will only update stats every so often
    # based on the intervals as updating is expensive.
    # We want to check on the main process state every time
    # as it's cheaper and we want this thread to be able
    # to exit quickly once the main process is done.
    num_intervals = 0
    while process.is_running():
        num_intervals += 1

        # Determine if there is anything to update, do so if so
        update_cpu = num_intervals % cpu_update_interval == 0
        update_memory = num_intervals % memory_update_interval == 0
        if update_cpu or update_memory:
            # Get current samples
            samples = await poll_process(pid, update_cpu, update_memory)

            # No samples, we're done
            if not samples:
                return result

            kill_reason: Optional[KilledReason] = None

            # Update max memory and check requirement, if any
            if update_memory:
                memory = sum(v.memory for v in samples if v.memory is not None)
                if memory > result.max_memory:
                    result.max_memory = memory
                    if max_memory and memory > max_memory:
                        kill_reason = KilledReason.MEMORY

            # Update percent cpu and check requirement, if any
            if update_cpu:
                percent_cpu = sum(sample_percent_cpu(v) for v in samples)
                if percent_cpu > result.max_percent_cpu:
                    result.max_percent_cpu = percent_cpu
                    if max_percent_cpu and percent_cpu > max_percent_cpu:
                        kill_reason = KilledReason.CPU

            # Kill if we have a reason to do so
            if kill_reason is not None:
                kill(kill_reason, samples)

            # Copy current samples for next cpu calculation
            if update_cpu:
                last_cpu_samples = {sample.pid: sample for sample in samples}

        # Poll
        await asyncio.sleep(poll_time)

    return result


def monitor_process(
    pid: int,
    poll_time: float,
    cpu_update_interval: int,
    memory_update_interval: int,
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
    cpu_update_interval : int
        How often to update CPU stats in number of polls.
    memory_update_interval : int
        How often to update memory stats in number of polls.
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
            cpu_update_interval=cpu_update_interval,
            memory_update_interval=memory_update_interval,
            max_percent_cpu=max_percent_cpu,
            max_memory=max_memory,
        )

    result_queue.put(asyncio.run(runner()))
