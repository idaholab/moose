# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement utilities for sampling per-process NVIDIA GPU memory with NVML."""

from collections import defaultdict
from contextlib import suppress
from typing import Optional

# Import rather than find_spec(), so that a broken install is also unavailable.
# Bind pynvml to None on failure (rather than leaving it unset) so the name is
# always patchable, e.g. by tests on a machine without pynvml installed.
try:
    import pynvml

    HAS_PYNVML = True
except ImportError:
    pynvml = None
    HAS_PYNVML = False
"""Whether or not the pynvml package (from nvidia-ml-py) is available."""


def nvidia_gpu_memory_unavailable() -> Optional[str]:
    """
    Get the reason that NVIDIA GPU memory cannot be sampled, if any.

    Returns:
    -------
    Optional[str]:
        The reason, or None if NvidiaGPUMemory can be used.

    """
    if not HAS_PYNVML:
        return "python package 'pynvml' (nvidia-ml-py) is not available"
    try:
        NvidiaGPUMemory().close()
    except (pynvml.NVMLError, RuntimeError) as e:
        return f"NVML: {e}"
    return None


class NvidiaGPUMemory:
    """Per-process GPU memory from NVML, summed over all visible devices."""

    def __init__(self):
        """Initialize NVML and get the device handles."""
        pynvml.nvmlInit()
        try:
            self._handles: list = [
                pynvml.nvmlDeviceGetHandleByIndex(i)
                for i in range(pynvml.nvmlDeviceGetCount())
            ]
            """The NVML device handles."""
            if not self._handles:
                raise RuntimeError("no NVIDIA GPUs found")
        except Exception:
            pynvml.nvmlShutdown()
            raise

    def sample(self) -> dict[int, int]:
        """
        Sample the GPU memory of all processes using a GPU.

        Processes whose memory NVML cannot report (usedGpuMemory is None) and
        devices that fail to report are skipped.

        Returns:
        -------
        dict[int, int]:
            Process ID to its GPU memory in bytes, summed over devices.

        """
        result = defaultdict(int)
        for handle in self._handles:
            with suppress(pynvml.NVMLError):
                for process in pynvml.nvmlDeviceGetComputeRunningProcesses(handle):
                    if process.usedGpuMemory is not None:
                        result[process.pid] += process.usedGpuMemory
        return dict(result)

    def close(self):
        """Shut down NVML."""
        with suppress(pynvml.NVMLError):
            pynvml.nvmlShutdown()
