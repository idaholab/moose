# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement utilities for monitoring a process' resource usage."""

import os
import sys
from collections import defaultdict
from multiprocessing import get_context
from multiprocessing.context import ForkProcess
from multiprocessing.managers import DictProxy
from typing import Optional

import psutil

MEMORY_PSS = sys.platform.startswith("linux") and os.path.exists(
    f"/proc/{os.getpid()}/smaps_rollup"
)
"""Whether or not to use PSS for memory tracking from smaps_rollup."""


def _parse_kb_fields(data: bytes) -> dict[bytes, int]:
    """
    Parse the "Field:  N kB" lines of a /proc/<pid>/smaps_rollup file.

    Arguments:
    ---------
    data : bytes
        The raw contents of the file.

    Returns:
    -------
    dict[bytes, int]:
        Field name (without the trailing colon) to its value in kB.

    """
    fields = {}
    for line in data.splitlines():
        # Skips the leading address-range line, which also contains a colon
        if line.endswith(b" kB"):
            key, _, value = line.partition(b":")
            fields[key] = int(value.split()[0])
    return fields


def _has_pss_file() -> bool:
    """Whether this kernel's smaps_rollup breaks out Pss_File (Linux 5.3+)."""
    with open(f"/proc/{os.getpid()}/smaps_rollup", "rb") as f:
        return b"Pss_File" in _parse_kb_fields(f.read())


MEMORY_PSS_EXCLUDE_FILE = MEMORY_PSS and _has_pss_file()
"""Whether or not to exclude file-backed pages (Pss_File) from the PSS memory count."""


def _rollup_memory(fields: dict[bytes, int], exclude_file: bool) -> Optional[int]:
    """
    Compute memory in bytes from parsed smaps_rollup fields.

    Pss (proportional set size) charges each process 1/N of every page shared by N
    processes, so memory shared between the MPI ranks of a test is not counted once
    per rank. That splitting also applies to file-backed pages, such as those of the
    moose/moose_test executable and its libraries, which every concurrently running
    test maps; their share therefore depends on how many other tests happen to be
    running. With exclude_file, return Pss - Pss_File (equivalently Pss_Anon +
    Pss_Shmem) to remove that share; otherwise return Pss.

    Returns None if a required field is missing, e.g. for an exiting process.

    Arguments:
    ---------
    fields : dict[bytes, int]
        The parsed smaps_rollup fields, from _parse_kb_fields().
    exclude_file : bool
        Whether to exclude Pss_File, which smaps_rollup provides on Linux 5.3+.

    Returns:
    -------
    Optional[int]:
        The memory in bytes, if available.

    """
    pss = fields.get(b"Pss")
    if pss is None:
        return None
    if exclude_file:
        pss_file = fields.get(b"Pss_File")
        if pss_file is None:
            return None
        pss -= pss_file
    return pss * 1024


def get_process_memory(process: psutil.Process) -> Optional[int]:
    """
    Get the memory of a process, if running.

    Prefer PSS (proportional set size) from smaps_rollup, excluding file-backed pages
    when the kernel reports them separately; see _rollup_memory().

    Arguments:
    ---------
    process : psutil.Process
        The process.

    Returns:
    -------
    int:
        The process memory in bytes, if available.

    """
    # Prefer PSS if available
    if MEMORY_PSS:
        try:
            with open(f"/proc/{process.pid}/smaps_rollup", "rb") as f:
                fields = _parse_kb_fields(f.read())
            return _rollup_memory(fields, MEMORY_PSS_EXCLUDE_FILE)
        except (FileNotFoundError, ProcessLookupError):
            pass
    # Otherwise, use RSS (will double count shared memory)
    else:
        try:
            return process.memory_info().rss
        except psutil.NoSuchProcess:
            pass

    return None


class MemoryMonitor:
    """Monitor that runs in a separate thread to track children memory usage."""

    def __init__(self, parent_pid: int, interval: float = 0.1):
        """Initialize state."""
        self._parent_pid: int = parent_pid
        """PID of the parent process to sample the children of."""

        self._interval: float = interval
        """How often to sample in seconds."""

        self._process: Optional[ForkProcess] = None
        """The Multiprocessing process."""

        self._manager = None
        """Manager for sharing data with the Multiprocessing process, once started."""

        self._samples: Optional[DictProxy[int, int]] = None
        """The shareable samples (child pid -> memory in bytes), once started."""

        self._stop_event = None
        """Event to stop the Multiprocessing process, once started."""

    def get_samples(self) -> dict[int, int]:
        """
        Get the last samples; child PID to cumulative memory usage in bytes.

        This returns a copy so that the dict can be accessed without
        locking (required for synchronization with the other thread).
        Thus, you should get a copy to this once and then use it many
        times as needed.
        """
        assert self._samples is not None
        return self._samples.copy()

    @staticmethod
    def _sampler(parent_pid: int, interval: float, stop_event, samples: DictProxy):
        """Run the sampler process; internal method called by start()."""
        while not stop_event.is_set():
            # Get the entire flattened process tree, removing process 0 as
            # it's the exit condition when searching recursively
            all_processes: dict[int, psutil.Process] = {
                p.info["pid"]: p for p in psutil.process_iter(["pid", "ppid"])
            }
            if 0 in all_processes:
                del all_processes[0]

            # Get the processes that are immediate children of the parent
            children_pids = set(
                pid for pid, p in all_processes.items() if p.info["ppid"] == parent_pid
            )

            # Recursively check if any of the processes in "children_pids"
            # are a parent of this process at any level
            def in_children_pids(p: psutil.Process) -> Optional[int]:
                ppid = p.info["ppid"]
                # Found a parent
                if ppid in children_pids:
                    return ppid
                # Recursively check
                return in_children_pids(pp) if (pp := all_processes.get(ppid)) else None

            # Accumulate memory from relevant processes
            result = defaultdict(int)
            for pid, p in all_processes.items():
                if (
                    ppid := pid if pid in children_pids else in_children_pids(p)
                ) is not None and (memory := get_process_memory(p)) is not None:
                    result[ppid] += memory

            # And update the shared state
            samples.clear()
            samples.update(result)

            # Wait, but wake early if stop_event is set
            stop_event.wait(interval)

    def start(self):
        """Start the sampler process."""
        assert self._process is None

        ctx = get_context("fork")
        self._manager = ctx.Manager()
        self._samples = self._manager.dict()
        self._stop_event = ctx.Event()
        self._process = ctx.Process(
            target=self._sampler,
            args=(
                self._parent_pid,
                self._interval,
                self._stop_event,
                self._samples,
            ),
            daemon=True,
        )
        assert self._process is not None
        self._process.start()

    def stop(self):
        """Stop the sampler process."""
        process = self._process
        if process is not None:
            assert self._stop_event is not None
            self._stop_event.set()
            process.join(timeout=1)
            if process.is_alive():
                process.terminate()
                process.join()
            self._process = None
            self._stop_event = None
