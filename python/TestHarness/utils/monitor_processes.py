# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement MonitorProcesses for monitoring a process' memory usage."""

import asyncio
import os
import sys
from contextlib import suppress
from dataclasses import dataclass, field
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
    """Single sample for a process' resource usage."""

    memory: int
    """Memory usage of the process in bytes, if sampled (otherwise 0)."""


@dataclass
class MonitoredProcess:
    """Result for a process monitored with monitor_process()."""

    max_memory: int = 0
    """The maximum memory used by the process in bytes."""
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
    def _get_process_memory(process: psutil.Process) -> int:
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
        int:
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

        return 0

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

            # Sample memory for all processes
            samples = {
                p.pid: ProcessSample(memory=cls._get_process_memory(p))
                for p in all_processes
            }

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
        process = self._processes.get(pid, MonitoredProcess())
        # Update max memory
        process.max_memory = max(process.max_memory, memory)
        # Update samples
        process.last_samples = samples

        return process
