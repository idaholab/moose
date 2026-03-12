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
from typing import Optional

import psutil

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


def get_processes_memory(parent_pids: set[int]) -> defaultdict[int, int]:
    """
    Get the estimated total memory for each parent process, including children.

    If a process is not running, it will not be included.

    Arguments:
    ---------
    parent_pids : set[int]
        The PIDs of the parent processes.

    Returns:
    -------
    defaultdict[int, int]:
        Mapping of parent PID -> estimated process memory in bytes.

    """
    # Get the entire flattened process tree
    all_processes: dict[int, psutil.Process] = {
        p.info["pid"]: p for p in psutil.process_iter(["pid", "ppid"])
    }

    # Recrusively check if any of the processes in "parent_pids"
    # are a parent of this process at any level
    def in_parent_pids(p: psutil.Process) -> Optional[int]:
        ppid = p.info["ppid"]
        # Found a parent
        if ppid in parent_pids:
            return ppid
        # Recursively check
        return in_parent_pids(pp) if (pp := all_processes.get(ppid)) else None

    # Accumulate memory from relevant processes
    result = defaultdict(int)
    for pid, p in all_processes.items():
        if (ppid := pid if pid in parent_pids else in_parent_pids(p)) is not None and (
            memory := get_process_memory(p)
        ) is not None:
            result[ppid] += memory
    return result
