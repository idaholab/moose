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
from contextlib import suppress
from importlib.util import find_spec
from multiprocessing import get_context
from multiprocessing.context import ForkProcess
from multiprocessing.managers import DictProxy
from typing import TYPE_CHECKING, Any, Optional

import psutil

HAS_PYNVML = find_spec("pynvml") is not None
"""Whether or not pynvml is available."""

# Load pynvml if available
if TYPE_CHECKING or HAS_PYNVML:
    import pynvml

MEMORY_PSS = sys.platform.startswith("linux") and os.path.exists(
    f"/proc/{os.getpid()}/smaps_rollup"
)
"""Whether or not to use PSS for memory tracking from smaps_rollup."""


def get_process_memory(process: psutil.Process) -> Optional[int]:
    """
    Get the memory of a process, if running.

    Prefer to use PSS (proportional set size) as the memory count if available.

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
                for line in f:
                    if line.startswith(b"Pss:"):
                        return int(line.split()[1]) * 1024  # b"Pss:   1234 kB"
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

    def __init__(self, parent_pid: int, track_nvidia: bool, interval: float = 0.1):
        """Initialize state."""
        self._parent_pid: int = parent_pid
        """PID of the parent process to sample the children of."""

        self._interval: float = interval
        """How often to sample in seconds."""

        self._track_nvidia: bool = track_nvidia
        """Whether or not to track Nvidia GPU memory usage."""

        self._process: Optional[ForkProcess] = None
        """The Multiprocessing process."""

        self._manager = None
        """Manager for sharing data with the Multiprocessing process, once started."""

        self._cpu_samples: Optional[DictProxy[int, int]] = None
        """The shareable CPU samples (child pid -> memory in bytes), once started."""

        self._gpu_samples: Optional[DictProxy[int, int]] = None
        """The shareable GPU samples (child pid -> memory in bytes), once started."""

        self._stop_event = None
        """Event to stop the Multiprocessing process, once started."""

        # If tracking nvidia GPUs, make sure we can actually
        # poll at least one GPU for memory usage
        if track_nvidia:
            if not HAS_PYNVML:
                print(
                    "WARNING: Not tracking GPU memory usage; "
                    "'pynvml' package unavailable"
                )
                self._track_nvidia = False
            else:
                self.init_pynvml_handles()
                pynvml.nvmlShutdown()

    @staticmethod
    def init_pynvml_handles() -> list:
        """Initialize pynvml and return the Nvidia GPU handlers, if possible."""
        pynvml.nvmlInit()

        handles = [
            pynvml.nvmlDeviceGetHandleByIndex(i)
            for i in range(pynvml.nvmlDeviceGetCount())
        ]

        if not handles:
            with suppress(pynvml.NVMLError):
                pynvml.nvmlShutdown()
            raise RuntimeError("Process monitoring failed to discover a Nvidia GPU")

        return handles

    def get_cpu_samples(self) -> dict[int, int]:
        """
        Get the last CPU samples; child PID to cumulative memory usage in bytes.

        This returns a copy so that the dict can be accessed without
        locking (required for synchronization with the other thread).
        Thus, you should get a copy to this once and then use it many
        times as needed.
        """
        assert self._cpu_samples is not None
        return self._cpu_samples.copy()

    def get_gpu_samples(self) -> Optional[dict[int, int]]:
        """
        Get the last GPU samples; child PID to cumulative memory usage in bytes.

        This returns a copy so that the dict can be accessed without
        locking (required for synchronization with the other thread).
        Thus, you should get a copy to this once and then use it many
        times as needed.
        """
        if not self._track_nvidia:
            return None
        assert self._gpu_samples is not None, "GPU samples not set"
        return self._gpu_samples.copy()

    @staticmethod
    def _sampler(
        parent_pid: int,
        interval: float,
        track_nvidia: bool,
        stop_event,
        cpu_samples: DictProxy,
        gpu_samples: DictProxy,
    ):
        """Run the sampler process; internal method called by start()."""
        # Load the pynvml handles if we have any GPUs available
        pynvml_handles = MemoryMonitor.init_pynvml_handles() if track_nvidia else None

        while not stop_event.is_set():
            # Accumulate nvidia GPU processes if we have handlers
            pynvml_processes: Optional[dict[int, Any]] = None
            if pynvml_handles is not None:
                pynvml_processes = {}
                for handle in pynvml_handles:
                    with suppress(pynvml.NVMLError):
                        pynvml_processes.update(
                            {
                                p.pid: p.usedGpuMemory
                                for p in pynvml.nvmlDeviceGetComputeRunningProcesses(
                                    handle
                                )
                            }
                        )

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
            cpu_result = defaultdict(int)
            gpu_result = defaultdict(int)
            for pid, p in all_processes.items():
                if (
                    ppid := pid if pid in children_pids else in_children_pids(p)
                ) is not None:
                    if (cpu_memory := get_process_memory(p)) is not None:
                        cpu_result[ppid] += cpu_memory
                    if (
                        pynvml_processes is not None
                        and (gpu_memory := pynvml_processes.get(p.pid)) is not None
                    ):
                        gpu_result[ppid] += gpu_memory

            # And update the shared state
            cpu_samples.clear()
            cpu_samples.update(cpu_result)
            if pynvml_handles is not None:
                gpu_samples.clear()
                gpu_samples.update(gpu_result)

            # Wait, but wake early if stop_event is set
            stop_event.wait(interval)

        # Shutdown our nvml reference if we had any handles
        if pynvml_handles is not None:
            with suppress(pynvml.NVMLError):
                pynvml.nvmlShutdown()

    def start(self):
        """Start the sampler process."""
        assert self._process is None

        ctx = get_context("fork")
        self._manager = ctx.Manager()
        self._cpu_samples = self._manager.dict()
        self._gpu_samples = self._manager.dict()
        self._stop_event = ctx.Event()
        self._process = ctx.Process(
            target=self._sampler,
            args=(
                self._parent_pid,
                self._interval,
                self._track_nvidia,
                self._stop_event,
                self._cpu_samples,
                self._gpu_samples,
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
