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
import time
from collections import defaultdict
from multiprocessing import get_context
from multiprocessing.context import ForkProcess
from multiprocessing.managers import NamespaceProxy, SyncManager
from typing import NamedTuple, Optional

import psutil
from TestHarness.utils.nvidia_gpu_memory import NvidiaGPUMemory

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


def _read_rollup(pid: int) -> dict[bytes, int]:
    """Read and parse /proc/<pid>/smaps_rollup; see _parse_kb_fields()."""
    with open(f"/proc/{pid}/smaps_rollup", "rb") as f:
        return _parse_kb_fields(f.read())


# Pss_File was added to smaps_rollup in Linux 5.3
MEMORY_PSS_EXCLUDE_FILE = MEMORY_PSS and b"Pss_File" in _read_rollup(os.getpid())
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
    # Without PSS, use RSS (will double count shared memory)
    if not MEMORY_PSS:
        try:
            return process.memory_info().rss
        except psutil.NoSuchProcess:
            return None

    try:
        fields = _read_rollup(process.pid)
    except (FileNotFoundError, ProcessLookupError):
        return None
    return _rollup_memory(fields, MEMORY_PSS_EXCLUDE_FILE)


class MemorySample(NamedTuple):
    """The memory of a child process and all of its descendants."""

    cpu: Optional[int]
    """The CPU memory in bytes, if any process's could be read."""
    gpu: Optional[int]
    """The NVIDIA GPU memory in bytes, if GPU memory is tracked."""


class MemoryMonitor:
    """Monitor that runs in a separate process to track children memory usage."""

    def __init__(self, parent_pid: int, interval: float = 0.1, track_gpu: bool = False):
        """Initialize state."""
        self._parent_pid: int = parent_pid
        """PID of the parent process to sample the children of."""

        self._interval: float = interval
        """How often to sample in seconds."""

        self._track_gpu: bool = track_gpu
        """Whether or not to also sample NVIDIA GPU memory."""

        self._process: Optional[ForkProcess] = None
        """The Multiprocessing process."""

        self._manager: Optional[SyncManager] = None
        """Manager for sharing data with the Multiprocessing process, once started."""

        self._shared: Optional[NamespaceProxy] = None
        """
        Shared state whose "samples" attribute is the latest samples (child pid ->
        MemorySample) and whose "gpu_error" attribute is the reason GPU memory
        sampling failed to start, if it did, once started.
        """

        self._stop_event = None
        """Event to stop the Multiprocessing process, once started."""

    def get_samples(self) -> dict[int, MemorySample]:
        """
        Get the last samples; child PID to cumulative memory usage.

        Each call fetches a copy from the sampler process, so you should get
        it once and then use it many times as needed.
        """
        assert self._shared is not None
        return self._shared.samples

    def get_gpu_error(self) -> Optional[str]:
        """
        Get the reason that GPU memory sampling failed to start in the sampler, if any.

        It is set before the first samples are published, after which GPU memory
        is reported as not tracked (None).
        """
        assert self._shared is not None
        return self._shared.gpu_error

    @staticmethod
    def _sampler(
        parent_pid: int,
        interval: float,
        track_gpu: bool,
        stop_event,
        shared: NamespaceProxy,
    ):
        """Run the sampler process; internal method called by start()."""
        # Initialized after the fork, as NVML state cannot be shared across one.
        # On failure, keep sampling CPU memory and report why GPU memory is not
        gpu_memory = None
        if track_gpu:
            try:
                gpu_memory = NvidiaGPUMemory()
            except Exception as e:
                shared.gpu_error = str(e)

        while not stop_event.is_set():
            start = time.monotonic()

            # Sample GPU memory before walking the process tree, so that a process
            # that NVML reports is normally also in the tree
            gpu_samples = gpu_memory.sample() if gpu_memory is not None else {}

            # Get the entire flattened process tree, removing process 0 as
            # it's the exit condition when walking up parents (on macOS, it
            # is listed as its own parent)
            all_processes: dict[int, psutil.Process] = {
                p.info["pid"]: p for p in psutil.process_iter(["pid", "ppid"])
            }
            if 0 in all_processes:
                del all_processes[0]

            def top_level_child(pid: int) -> Optional[int]:
                """Get the direct child of parent_pid that pid is or descends from."""
                while (ppid := all_processes[pid].info["ppid"]) != parent_pid:
                    if ppid not in all_processes:
                        return None
                    pid = ppid
                return pid

            # Accumulate the memory of each direct child and all of its descendants
            cpu_result: dict[int, int] = defaultdict(int)
            gpu_result: dict[int, int] = defaultdict(int)
            for pid, p in all_processes.items():
                if (child := top_level_child(pid)) is None:
                    continue
                # Counted separately, so that a process whose CPU memory cannot be
                # read (e.g. while exiting) still contributes its GPU memory
                if (memory := get_process_memory(p)) is not None:
                    cpu_result[child] += memory
                if (pid_gpu_memory := gpu_samples.get(pid)) is not None:
                    gpu_result[child] += pid_gpu_memory

            # And update the shared state; a single assignment, so a reader never
            # sees a partially updated set of samples
            shared.samples = {
                child: MemorySample(
                    cpu_result.get(child),
                    gpu_result.get(child, 0) if gpu_memory is not None else None,
                )
                for child in cpu_result.keys() | gpu_result.keys()
            }

            # Wait out the rest of the interval, so that a slow pass (many or
            # large processes) does not lengthen the sampling period; wake early
            # if stop_event is set
            stop_event.wait(max(0.0, interval - (time.monotonic() - start)))

        if gpu_memory is not None:
            gpu_memory.close()

    def start(self):
        """Start the sampler process."""
        assert self._process is None

        ctx = get_context("fork")
        self._manager = ctx.Manager()
        self._shared = self._manager.Namespace(samples={}, gpu_error=None)
        self._stop_event = ctx.Event()
        self._process = ctx.Process(
            target=self._sampler,
            args=(
                self._parent_pid,
                self._interval,
                self._track_gpu,
                self._stop_event,
                self._shared,
            ),
            daemon=True,
        )
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
        if self._manager is not None:
            self._manager.shutdown()
            self._manager = None
            self._shared = None
