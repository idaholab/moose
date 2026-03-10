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
from contextlib import suppress
from typing import TYPE_CHECKING, Iterable, Optional

if TYPE_CHECKING:
    import psutil
else:
    from importlib.util import find_spec

    if find_spec("psutil") is not None:
        import psutil
    else:
        raise ModuleNotFoundError("Requires the 'psutil' package")

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
            return int(data[start:end]) * 1024

    # Otherwise, use RSS (will double count shared memory)
    else:
        with suppress(psutil.NoSuchProcess):
            return process.memory_info().rss

    return None


def get_processes_memory(pids: Iterable[int]) -> dict[int, int]:
    """
    Get the estimated total memory for each process, including children.

    If a process is not running, it will not be included.

    Arguments:
    ---------
    pids : Iterable[int]
        The IDs of the processes.

    Returns:
    -------
    dict[int, int]:
        Mapping of pid -> estimated process memory.

    """

    def recursive_memory(pid: int) -> Optional[int]:
        try:
            p = psutil.Process(pid)
        except psutil.NoSuchProcess:
            return None

        ps: list[psutil.Process] = [p]
        with suppress(psutil.NoSuchProcess):
            ps += p.children(recursive=True)

        if memory := sum([memory for p in ps if (memory := get_process_memory(p))]):
            return memory

        return None

    return {pid: memory for pid in pids if (memory := recursive_memory(pid))}
