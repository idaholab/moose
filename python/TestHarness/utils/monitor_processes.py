# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement MonitorProcesses for monitoring a process cpu and memory usage."""

import asyncio
import os
import sys
from contextlib import suppress
from dataclasses import dataclass, field
from time import perf_counter
from typing import TYPE_CHECKING, Iterable, Optional

if TYPE_CHECKING:
    import psutil
else:
    from importlib.util import find_spec

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
    """Wall time at which the cpu sample was made in seconds."""
    cpu_time: float
    """Total CPU time (user + system) of the process in seconds."""
    memory: int
    """Memory usage of the process in bytes, if sampled (otherwise 0)."""


@dataclass
class MonitoredProcess:
    """Result for a process monitored with monitor_process()."""

    max_memory: int = 0
    """The estimated maximum memory of the process and children in bytes."""
    max_percent_cpu: float = 0
    """The maximum percent CPU of the process and children."""
    last_samples: dict[int, ProcessSample] = field(default_factory=dict)
    """The last samples for the process, if any."""


class MonitorProcesses:
    """Utility that monitors usage of processes."""

    def __init__(self):
        """Initialize state."""
        self._processes: dict[int, MonitoredProcess] = {}
        """The currently monitored processes (pid -> MonitoredProcess)."""

    @property
    def processes(self) -> dict[int, MonitoredProcess]:
        """Get the currently monitored processes."""
        return self._processes

    def get(self, pid: int) -> Optional[MonitoredProcess]:
        """Get a monitored process by process ID."""
        return self.processes.get(pid)

    def update(self, pids: Iterable[int]):
        """
        Update the given processes.

        Arguments:
        ---------
        pids : Iterable[int]
            The process IDs of the processes to update.

        """

        async def async_sample_processes():
            return await self._sample_processes(pids)

        # Sample the processes
        samples = asyncio.run(async_sample_processes())

        # Build/update the MonitoredProcess for each process
        self.processes.update(
            {
                pid: self._build_monitored_process(pid, samples)
                for pid, samples in samples.items()
            }
        )

    @staticmethod
    def _get_psutil_process(pid: int) -> Optional[psutil.Process]:
        """Get the psutil process if it is available."""
        try:
            return psutil.Process(pid)
        except psutil.NoSuchProcess:
            return None

    @staticmethod
    def _get_process_and_children(process: psutil.Process) -> list[psutil.Process]:
        """
        Get the parent + recursive children of a single psutil process.

        Arguments:
        ---------
        process : psutil.Process
            The process.

        Returns:
        -------
        list[psutil.Process]:
            The process plus all of its children.

        """
        processes: list[psutil.Process] = [process]
        with suppress(psutil.NoSuchProcess):
            processes += process.children(recursive=True)
        return processes

    @staticmethod
    def _get_process_cpu_time(process: psutil.Process) -> Optional[float]:
        """
        Get the cpu time of a process (user + system), if running.

        Arguments:
        ---------
        process : psutil.Process
            The process.

        Returns:
        -------
        Optional[float]:
            The user + system time, if running.

        """
        if process.is_running():
            with suppress(psutil.NoSuchProcess):
                times = process.cpu_times()
                return times.user + times.system
        return None

    @staticmethod
    def _get_process_memory(process: psutil.Process) -> Optional[int]:
        """
        Get the memory of a process, if running.

        With prefer to use PSS (proportional set size) as the memory
        count if available.

        Arguments:
        ---------
        process : psutil.Process
            The process.

        Returns:
        -------
        Optional[int]:
            The process memory in kB, if available.

        """
        # Prefer PSS if available
        if MEMORY_PSS:
            data = None
            with (
                suppress(FileNotFoundError),
                suppress(ProcessLookupError),
                open(f"/proc/{process.pid}/smaps_rollup", "rb") as f,
            ):
                data = f.read()
            if data is not None and (idx := data.find(b"Pss:")) != -1:
                start = idx + 4
                while data[start] == 32:
                    start += 1
                end = start
                while data[end] != 32:
                    end += 1
                return int(data[start:end]) * 1000

        # Otherwise, use RSS (will double count shared memory)
        else:
            with suppress(psutil.NoSuchProcess):
                return process.memory_info().rss

        return None

    @classmethod
    async def _sample_processes(
        cls,
        pids: Iterable[int],
    ) -> dict[int, dict[int, ProcessSample]]:
        """
        Sample usage for the given processes, including their children.

        This is a heaver call so it is asynchronous.

        Arguments:
        ---------
        pids : Iterable[int]
            The IDs of each process to sample.

        Returns:
        -------
        dict[int, dict[int, ProcessSample]]:
            Map of parent process ID to samples for each parent+children process.

        """

        # Helper for sampling a single process
        def _sample_process(pid: int) -> Optional[dict[int, ProcessSample]]:
            process = cls._get_psutil_process(pid)

            # Process has ended
            if process is None:
                return None

            # Process + all of its recursive children
            all_processes = cls._get_process_and_children(process)

            # Sample all processes
            samples: dict[int, ProcessSample] = {}
            wall_time = perf_counter()
            for p in all_processes:
                # Get cpu time, or continue if not running
                if (cpu_time := cls._get_process_cpu_time(process)) is None:
                    continue
                # Sample memory
                memory = cls._get_process_memory(p)

                # Add sample
                samples[p.pid] = ProcessSample(
                    pid=p.pid,
                    wall_time=wall_time,
                    cpu_time=cpu_time,
                    memory=memory if memory is not None else 0,
                )

            return samples if samples else None

        def _sample_processes() -> dict[int, dict[int, ProcessSample]]:
            return {
                pid: pid_samples
                for pid in pids
                if (pid_samples := _sample_process(pid))
            }

        # Dispatch to another thread (this is a heavier call)
        try:
            return await asyncio.to_thread(_sample_processes)
        except RuntimeError as e:
            # During interpreter shutdown, ThreadPoolExecutor forbids new tasks.
            # Treat that as "we're done monitoring" and return no samples.
            if "interpreter shutdown" in str(e) or "Event loop is closed" in str(e):
                return {}
            raise
        except asyncio.CancelledError:
            return {}

    def _build_monitored_process(
        self,
        pid: int,
        samples: dict[int, ProcessSample],
    ) -> MonitoredProcess:
        """
        Build or update a MonitoredProcess given a process' new samples.

        Arguments:
        ---------
        pid : int
            The ID of the parent process.
        samples : dict[int, ProcessSample]
            The samples for the process and its children.

        Returns:
        -------
        MonitoredProcess:
            The updated MonitoredProcess.

        """
        # Cumulative memory usage across all ranks
        memory = sum([sample.memory for sample in samples.values()], 0)

        # Get previous MonitoredProcess if we've sampled it before
        process = self._processes.get(pid)

        # Haven't seen this process before; initialize it
        if process is None:
            process = MonitoredProcess()
        # Have previously seen the process so we can get a
        # CPU percentage (requires a previous sample)
        else:
            cpu = 0.0
            for pid, sample in samples.items():
                last_sample = process.last_samples.get(pid)
                if last_sample is not None:
                    cpu_time_diff = sample.cpu_time - last_sample.cpu_time
                    wall_time_diff = sample.wall_time - last_sample.wall_time
                    cpu += cpu_time_diff / wall_time_diff
            process.max_percent_cpu = max(process.max_percent_cpu, 100.0 * cpu)

        # Update max memory
        process.max_memory = max(process.max_memory, memory)
        # Update samples
        process.last_samples = samples

        return process
