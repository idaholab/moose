# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test RunLocal.monitorJobProcesses()."""

import unittest
from types import SimpleNamespace
from typing import Optional
from unittest.mock import Mock, patch

from TestHarness.runners.Runner import Runner
from TestHarness.schedulers.RunLocal import RunLocal
from TestHarness.utils.monitor_processes import MemorySample

MIB = 2**20


def build_job(slots: int = 1) -> Mock:
    """Build a running job with a real Runner, so that peaks are tracked."""
    job = Mock()
    job.error = "error"
    job.getStatus.return_value = "running"
    job.getSlots.return_value = slots
    job.getRunner.return_value = Runner(job, Mock(), Mock())
    return job


def build_scheduler(
    max_memory: Optional[float] = None,
    max_gpu_memory: Optional[float] = None,
    gpu_error: Optional[str] = None,
) -> RunLocal:
    """Build a RunLocal with a fake monitor, without running its constructor."""
    scheduler = RunLocal.__new__(RunLocal)
    scheduler.options = SimpleNamespace(
        max_memory_per_slot=max_memory,
        max_gpu_memory_per_slot=max_gpu_memory,
        colored=False,
    )
    scheduler.scheduler_options = SimpleNamespace(monitor_job_gpu_memory=True)
    scheduler._checked_gpu_error = False
    scheduler._memory_monitor = Mock(get_gpu_error=Mock(return_value=gpu_error))
    return scheduler


def monitor(
    jobs: dict[int, Mock],
    samples: dict[int, MemorySample],
    scheduler: Optional[RunLocal] = None,
    **kwargs,
):
    """
    Run RunLocal.monitorJobProcesses() for the given jobs and samples.

    The scheduler is built with build_scheduler(**kwargs) if not given.
    """
    if scheduler is None:
        scheduler = build_scheduler(**kwargs)
    scheduler._memory_monitor.get_samples.return_value = samples
    with patch.object(RunLocal, "getActiveJobPIDMap", return_value=jobs):
        scheduler.monitorJobProcesses()


class TestRunLocalMonitorJobProcesses(unittest.TestCase):
    """Test RunLocal.monitorJobProcesses()."""

    def testPeaks(self):
        """Test that the CPU and GPU peaks are updated."""
        job = build_job()
        monitor({1: job}, {1: MemorySample(10 * MIB, 20 * MIB)})
        monitor({1: job}, {1: MemorySample(5 * MIB, 30 * MIB)})
        self.assertEqual(job.getRunner().max_memory, 10 * MIB)
        self.assertEqual(job.getRunner().max_gpu_memory, 30 * MIB)
        job.killProcess.assert_not_called()

    def testUnknownCPU(self):
        """Test that a sample with unknown CPU memory still updates the GPU peak."""
        job = build_job()
        monitor({1: job}, {1: MemorySample(10 * MIB, 20 * MIB)})
        monitor({1: job}, {1: MemorySample(None, 30 * MIB)}, max_memory=1)
        self.assertEqual(job.getRunner().max_memory, 10 * MIB)
        self.assertEqual(job.getRunner().max_gpu_memory, 30 * MIB)
        job.killProcess.assert_not_called()

    def testNoGPU(self):
        """Test that the GPU peak is not set when GPU memory is not tracked."""
        job = build_job()
        monitor({1: job}, {1: MemorySample(10 * MIB, None)}, max_gpu_memory=1)
        self.assertEqual(job.getRunner().max_memory, 10 * MIB)
        self.assertIsNone(job.getRunner().max_gpu_memory)
        job.killProcess.assert_not_called()

    def testAllJobsChecked(self):
        """Test that a job whose peak did not increase does not end the pass."""
        first, second = build_job(), build_job()
        monitor({1: first}, {1: MemorySample(10 * MIB, 10 * MIB)})
        monitor(
            {1: first, 2: second},
            {1: MemorySample(10 * MIB, 10 * MIB), 2: MemorySample(1, 50 * MIB)},
            max_gpu_memory=20,
        )
        first.killProcess.assert_not_called()
        second.killProcess.assert_called_once()

    def testCPULimit(self):
        """Test killing a job over the CPU memory limit, per slot."""
        within, over = build_job(slots=2), build_job()
        monitor(
            {1: within, 2: over},
            {1: MemorySample(30 * MIB, None), 2: MemorySample(30 * MIB, None)},
            max_memory=20,
        )
        within.killProcess.assert_not_called()
        over.killProcess.assert_called_once_with(
            "error",
            "KILLED: OVER MEMORY",
            "JOB KILLED (OVER MEMORY): Memory/slot 30.00 MB > allowed 20.00 MB",
        )

    def testGPULimit(self):
        """Test killing a job over the GPU memory limit, per slot."""
        within, over = build_job(slots=2), build_job()
        monitor(
            {1: within, 2: over},
            {1: MemorySample(1, 30 * MIB), 2: MemorySample(1, 30 * MIB)},
            max_memory=20,
            max_gpu_memory=20,
        )
        within.killProcess.assert_not_called()
        over.killProcess.assert_called_once_with(
            "error",
            "KILLED: OVER GPU MEMORY",
            "JOB KILLED (OVER GPU MEMORY): GPU memory/slot 30.00 MB > allowed 20.00 MB",
        )

    def testErrorSkipped(self):
        """Test that a job already in error (e.g. killed) is not updated."""
        job = build_job()
        job.getStatus.return_value = job.error
        monitor({1: job}, {1: MemorySample(30 * MIB, 30 * MIB)}, max_gpu_memory=1)
        self.assertIsNone(job.getRunner().max_memory)
        job.killProcess.assert_not_called()

    def testGPUError(self):
        """Test that a GPU sampling failure is reported once."""
        scheduler = build_scheduler(gpu_error="NVML failed")
        job = build_job()
        sample = {1: MemorySample(10 * MIB, None)}
        with patch("TestHarness.schedulers.RunLocal.printPrefixed") as printed:
            # Not checked until samples are available
            monitor({1: job}, {}, scheduler)
            printed.assert_not_called()

            monitor({1: job}, sample, scheduler)
            monitor({1: job}, sample, scheduler)
        printed.assert_called_once_with(
            "WARNING",
            "GPU memory is not tracked; sampling failed to start: NVML failed",
            color=None,
        )
        scheduler._memory_monitor.get_gpu_error.assert_called_once()


if __name__ == "__main__":
    unittest.main()
