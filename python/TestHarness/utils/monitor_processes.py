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
from typing import TYPE_CHECKING, Optional

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


def get_processes_memory(pids: set[int]) -> defaultdict[int, int]:
    """
    Get the estimated total memory for each process, including children.

    The children of the parent processes (the ones in "pids") must
    have their process group IDs set to that of the parent. This
    enables loading all processes at once, searching by process
    group ID instead of needing to recurse through the process
    tree for children for each parent process. This is true
    in the SubprocessRunner, as we set preexec_fn=os.setsid
    when spawning the parent subprocess.

    If a process is not running, it will not be included.

    Arguments:
    ---------
    pids : Iterable[int]
        The PIDs of the parent processes.

    Returns:
    -------
    defaultdict[int, int]:
        Mapping of parent PID -> estimated process memory in bytes.

    """

    result = defaultdict(int)

    for p in psutil.process_iter(["pid"]):
        try:
            pgid = os.getpgid(p.info["pid"])
        except ProcessLookupError:
            continue
        if pgid not in pids:
            continue
        if (memory := get_process_memory(p)) is not None:
            result[pgid] += memory

    return result
