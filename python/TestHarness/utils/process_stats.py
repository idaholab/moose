# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement a utility for tracking a process' CPU and memory usage."""

import os
import sys
from contextlib import suppress
from dataclasses import dataclass
from importlib.util import find_spec
from time import perf_counter, sleep
from typing import TYPE_CHECKING, Iterator, Optional

# Try to load psutil; not a strict requirement but
# enables tracking memory
if TYPE_CHECKING:
    import psutil
else:
    if find_spec("psutil") is not None:
        import psutil
    else:
        psutil = None

MEMORY_PSS = sys.platform.startswith("linux") and os.path.exists(
    f"/proc/{os.getpid()}/smaps_rollup"
)
"""Whether or not to use PSS for memory tracking from smaps_rollup."""


def get_memory(process: psutil.Process) -> Optional[int]:
    """Get an approximation for a process' total memory in bytes."""
    # Use proportional set size (PSS) if available, which is
    # a better approximation for MPI processes
    if MEMORY_PSS:
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


def get_cpu_time(process: psutil.Process) -> Optional[float]:
    """Get the CPU time (user + system) in seconds of the given process."""
    with suppress(psutil.NoSuchProcess):
        times = process.cpu_times()
        return times.user + times.system
    return None


@dataclass
class CPUSample:
    """Data class for a single CPU time sample."""

    wall_time: float
    """Wall time of the sample in seconds."""
    cpu_time: float
    """CPU time at the time of sample in seconds."""


class TrackedProcess:
    """Storage for a process whose statistics are tracked."""

    def __init__(self, process: psutil.Process):
        """Initialize state."""
        self._process: psutil.Process = process
        """The process to track."""

        self._last_cpu: Optional[CPUSample] = None
        """The last CPU time sample."""

        self._current_cpu: Optional[CPUSample] = None
        """The current CPU time sample."""

        self._max_memory: int = 0
        """Maximum process memory found in bytes."""

        self._done: bool = False
        """Whether or not the process was found to be done."""

    @property
    def process(self) -> psutil.Process:
        """Get the tracked process."""
        return self._process

    @property
    def done(self) -> bool:
        """Whether or not the process was found to be done."""
        return self._done

    def update(self):
        """Update the statistics."""
        assert not self.done

        wall_time = perf_counter()
        if cpu_time := get_cpu_time(self.process):
            current = CPUSample(wall_time=wall_time, cpu_time=cpu_time)
            self._last_cpu = self._current_cpu
            self._current_cpu = current

            memory = get_memory(self.process)
            if memory and memory > self._max_memory:
                self._max_memory = memory
        else:
            self._done = True

    @property
    def current_percent_cpu(self) -> float:
        """
        Get the current percent CPU usage.

        Uses the last two samples.
        """
        last = self._last_cpu
        if last is None:
            return 0.0

        current = self._current_cpu
        assert current is not None
        return (
            (current.cpu_time - last.cpu_time)
            / (current.wall_time - last.wall_time)
            * 100
        )


@dataclass
class ProcessStats:
    """Stores a process' statistics, including children."""

    cpu_percent: float
    """Percent CPU usage of the process and children over the last interval."""

    max_memory: int
    """Maximum memory usage in bytes of the process and children since start."""


def process_stats(
    pid: int, poll_time: float, update_interval: int
) -> Iterator[ProcessStats]:
    """
    Obtain rolling process statistics.

    Includes statistics for all of the process' children.

    Arguments:
    ---------
    pid : int
        The PID of the top-level process.
    poll_time : float
        How often to poll for the top-level process' existance.
    update_interval : int
        The number of polls between stat updates.

    """
    assert isinstance(pid, int)
    assert isinstance(poll_time, float)
    assert isinstance(update_interval, int)
    assert update_interval > 0

    # Tracked processes that are running; pid -> data
    running: dict[int, TrackedProcess] = {}

    # Capture the psutil process
    try:
        process = psutil.Process(pid)
    # Doesn't exist; nothing else we can do ehre
    except psutil.NoSuchProcess:
        return

    num_intervals = 0
    while process.is_running():
        num_intervals += 1

        if num_intervals == update_interval:
            # Reset intervals
            num_intervals = 0

            # Collect parent + children processes
            all_processes = [process]
            with suppress(psutil.NoSuchProcess):
                all_processes += process.children(recursive=True)

            # Add any new processes to running
            for p in all_processes:
                if p.pid not in running:
                    running[p.pid] = TrackedProcess(p)

            # Update processes and possibly finish
            finished_pids = []
            for pid, data in running.items():
                data.update()
                if data.done:
                    # self._finished.append(data)
                    finished_pids.append(pid)

            # Remove processes that are no longer running
            for pid in finished_pids:
                del running[pid]

            # If every process is done, there's nothing left to do
            if not running:
                break

            yield ProcessStats(
                cpu_percent=sum(v.current_percent_cpu for v in running.values()),
                max_memory=sum(v._max_memory for v in running.values()),
            )

        sleep(poll_time)
