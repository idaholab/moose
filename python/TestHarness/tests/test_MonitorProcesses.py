# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.utils.monitor_processes."""

import os
import subprocess
import sys
import tempfile
import time
import unittest
from contextlib import contextmanager
from typing import Iterator, Optional
from unittest.mock import Mock, mock_open, patch

import psutil

from TestHarness.utils.monitor_processes import (
    MEMORY_PSS,
    MEMORY_PSS_EXCLUDE_FILE,
    MemoryMonitor,
    _parse_kb_fields,
    _read_rollup,
    _rollup_memory,
    get_process_memory,
)

MIB = 1024 * 1024

# Memory the child processes below allocate; large next to the few MiB an idle
# interpreter uses, so that the effect being tested dominates any noise
SIZE = 64 * MIB

# Allowed deviation of a measured memory difference from its expected value.
# Observed deviations are under 0.5 MiB (Linux 5.14); this leaves room for
# interpreter and allocator differences between child processes while staying
# well below SIZE / 2, so counting file-backed memory or counting shared memory
# once per process is still detected.
TOLERANCE = 8 * MIB

# Script for a child process that sets up memory according to argv[1], prints the
# space-separated PIDs of any processes it forked once all are ready, and then
# exits once stdin is closed. Modes:
#   none:   nothing beyond the interpreter (a baseline)
#   anon:   argv[2] bytes of private anonymous memory
#   file:   a read-only mapping of the file argv[3], argv[2] bytes long
#   shared: argv[2] bytes of shared anonymous memory, mapped by this process and
#           one forked process
#   tree:   one forked process that allocates argv[2] bytes of anonymous memory
CHILD = """
import mmap, os, sys

mode, size, path = sys.argv[1], int(sys.argv[2]), sys.argv[3]
page = mmap.PAGESIZE

def touch(data):
    # Write one byte per page so that every page is resident
    data[::page] = bytes([1]) * len(range(0, len(data), page))

data = None
if mode == "anon":
    data = bytearray(size)
    touch(data)
elif mode == "shared":
    data = mmap.mmap(-1, size)
    touch(data)
elif mode == "file":
    with open(path, "rb") as f:
        data = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
    data[::page]

forked = []
if mode in ("shared", "tree"):
    ready_read, ready_write = os.pipe()
    pid = os.fork()
    if pid == 0:
        if mode == "shared":
            # Map the shared pages into this process too
            data[::page]
        else:
            data = bytearray(size)
            touch(data)
        os.write(ready_write, b"1")
        sys.stdin.read()
        os._exit(0)
    os.read(ready_read, 1)
    forked.append(pid)

print(*forked, flush=True)
sys.stdin.read()
for pid in forked:
    os.waitpid(pid, 0)
"""

# A smaps_rollup sample as produced on Linux 5.3+, which breaks Pss down into
# Pss_Anon/Pss_File/Pss_Shmem.
ROLLUP_WITH_PSS_FILE = b"""\
55d0a9097000-7fff20d3c000 ---p 00000000 00:00 0                          [rollup]
Rss:                1564 kB
Pss:                 198 kB
Pss_Anon:            104 kB
Pss_File:             94 kB
Pss_Shmem:             0 kB
Shared_Clean:       1424 kB
Shared_Dirty:          0 kB
Private_Clean:        36 kB
Private_Dirty:       104 kB
Swap:                  0 kB
SwapPss:               0 kB
"""

# A smaps_rollup sample as produced before Linux 5.3, with no Pss_Anon/Pss_File/
# Pss_Shmem breakdown.
ROLLUP_WITHOUT_PSS_FILE = b"""\
55d0a9097000-7fff20d3c000 ---p 00000000 00:00 0                          [rollup]
Rss:                1564 kB
Pss:                 198 kB
Shared_Clean:       1424 kB
Shared_Dirty:          0 kB
Private_Clean:        36 kB
Private_Dirty:       104 kB
Swap:                  0 kB
SwapPss:               0 kB
"""


class TestParseKbFields(unittest.TestCase):
    """Test _parse_kb_fields()."""

    def testRollup(self):
        """Test that all "Field:  N kB" lines of a smaps_rollup sample are parsed."""
        fields = _parse_kb_fields(ROLLUP_WITH_PSS_FILE)
        self.assertEqual(fields[b"Pss"], 198)
        self.assertEqual(fields[b"Pss_Anon"], 104)
        self.assertEqual(fields[b"Pss_File"], 94)
        self.assertEqual(fields[b"Pss_Shmem"], 0)
        # The leading address-range line is not a field
        self.assertEqual(len(fields), 11)


class TestRollupMemory(unittest.TestCase):
    """Test _rollup_memory()."""

    def testExcludeFile(self):
        """Test Pss - Pss_File is returned when excluding file-backed pages."""
        fields = _parse_kb_fields(ROLLUP_WITH_PSS_FILE)
        # Pss - Pss_File = 198 - 94 = 104 kB
        self.assertEqual(_rollup_memory(fields, True), 104 * 1024)

    def testIncludeFile(self):
        """Test Pss is returned when not excluding file-backed pages."""
        fields = _parse_kb_fields(ROLLUP_WITHOUT_PSS_FILE)
        self.assertEqual(_rollup_memory(fields, False), 198 * 1024)

    def testMissingFields(self):
        """Test None is returned when a required field is missing."""
        self.assertIsNone(_rollup_memory({}, False))
        fields = _parse_kb_fields(ROLLUP_WITHOUT_PSS_FILE)
        self.assertIsNone(_rollup_memory(fields, True))


class TestGetProcessMemory(unittest.TestCase):
    """Test get_process_memory() with smaps_rollup."""

    def _getProcessMemory(self, rollup: bytes) -> Optional[int]:
        process = Mock()
        process.pid = 1234
        with patch("builtins.open", mock_open(read_data=rollup)) as mocked_open:
            memory = get_process_memory(process)
        mocked_open.assert_called_once_with("/proc/1234/smaps_rollup", "rb")
        return memory

    @patch("TestHarness.utils.monitor_processes.MEMORY_PSS", True)
    @patch("TestHarness.utils.monitor_processes.MEMORY_PSS_EXCLUDE_FILE", True)
    def testExcludeFile(self):
        """Test Pss - Pss_File is used on kernels that report Pss_File."""
        self.assertEqual(self._getProcessMemory(ROLLUP_WITH_PSS_FILE), 104 * 1024)

    @patch("TestHarness.utils.monitor_processes.MEMORY_PSS", True)
    @patch("TestHarness.utils.monitor_processes.MEMORY_PSS_EXCLUDE_FILE", False)
    def testIncludeFile(self):
        """Test Pss is used on kernels that do not report Pss_File."""
        self.assertEqual(self._getProcessMemory(ROLLUP_WITHOUT_PSS_FILE), 198 * 1024)


@contextmanager
def run_child(mode: str, size: int = 0, path: str = "") -> Iterator[list[int]]:
    """
    Run CHILD in the given mode until the context exits.

    Yields the PIDs of the child followed by those of any processes it forked,
    once all of them have set up their memory.
    """
    process = subprocess.Popen(
        [sys.executable, "-c", CHILD, mode, str(size), path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        text=True,
    )
    try:
        assert process.stdout is not None
        forked = [int(pid) for pid in process.stdout.readline().split()]
        yield [process.pid, *forked]
    finally:
        assert process.stdin is not None
        process.stdin.close()
        process.wait(timeout=10)


def measure(pid: int) -> int:
    """Measure the memory of a running process with get_process_memory()."""
    memory = get_process_memory(psutil.Process(pid))
    assert memory is not None
    return memory


def is_tmpfs(path: str) -> bool:
    """Whether the given path is on a tmpfs, whose pages Linux counts as shmem."""
    mounts = [
        p
        for p in psutil.disk_partitions(all=True)
        if os.path.commonpath([path, p.mountpoint]) == p.mountpoint
    ]
    return max(mounts, key=lambda p: len(p.mountpoint)).fstype == "tmpfs"


class TestProcessMemory(unittest.TestCase):
    """Test get_process_memory() on real processes."""

    def setUp(self):
        """Measure a child that allocates nothing, to compare the others against."""
        with run_child("none") as (pid,):
            self.baseline = measure(pid)

    def testAnonymous(self):
        """Test that private anonymous memory is counted."""
        with run_child("anon", SIZE) as (pid,):
            memory = measure(pid)
        self.assertAlmostEqual(memory - self.baseline, SIZE, delta=TOLERANCE)

    @unittest.skipUnless(MEMORY_PSS_EXCLUDE_FILE, "requires Pss_File (Linux 5.3+)")
    def testFileExcluded(self):
        """Test that the pages of a mapped file are not counted."""
        # Not in /tmp, which may be a tmpfs; Linux counts tmpfs pages as Pss_Shmem
        test_dir = os.path.dirname(os.path.abspath(__file__))
        if is_tmpfs(test_dir):
            self.skipTest(f"{test_dir} is on a tmpfs")

        with tempfile.NamedTemporaryFile(dir=test_dir) as f:
            f.write(bytes(SIZE))
            f.flush()
            with run_child("file", SIZE, f.name) as (pid,):
                memory = measure(pid)
                pss = _read_rollup(pid)[b"Pss"] * 1024

        self.assertAlmostEqual(memory, self.baseline, delta=TOLERANCE)
        # Check that the file's pages were resident, and so would have been
        # counted had they not been excluded
        self.assertGreater(pss - memory, SIZE - TOLERANCE)

    @unittest.skipUnless(MEMORY_PSS, "requires smaps_rollup (Linux 4.14+)")
    def testSharedProportional(self):
        """Test that memory shared by two processes is split between them."""
        with run_child("shared", SIZE) as (pid, forked_pid):
            memory = measure(pid)
            forked_memory = measure(forked_pid)

        # The forked process also shares the interpreter's pages with its parent,
        # so the pair is charged one baseline and the shared memory once in total
        self.assertAlmostEqual(
            memory + forked_memory - self.baseline, SIZE, delta=TOLERANCE
        )
        for each in (memory, forked_memory):
            self.assertAlmostEqual(each, SIZE / 2 + self.baseline / 2, delta=TOLERANCE)


class TestMemoryMonitor(unittest.TestCase):
    """Test MemoryMonitor on real processes."""

    def testTree(self):
        """Test that the memory of a child's descendants is added to the child."""
        monitor = MemoryMonitor(os.getpid(), interval=0.05)
        monitor.start()
        try:
            with run_child("tree", SIZE) as (pid, forked_pid):
                expected = measure(pid) + measure(forked_pid)
                # Wait for a sample taken after the forked process allocated;
                # sampling takes well under a second, so 10 s is generous
                deadline = time.monotonic() + 10
                while (samples := monitor.get_samples()).get(pid, 0) < SIZE:
                    self.assertLess(time.monotonic(), deadline)
                    time.sleep(0.05)
        finally:
            monitor.stop()

        # Samples are keyed by direct children of the monitored process only, so
        # the forked process is not sampled on its own
        self.assertNotIn(forked_pid, samples)
        self.assertAlmostEqual(samples[pid], expected, delta=TOLERANCE)


if __name__ == "__main__":
    unittest.main()
