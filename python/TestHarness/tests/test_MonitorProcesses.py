# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.utils.monitor_processes."""

import sys
import unittest
from typing import Optional
from unittest.mock import Mock, mock_open, patch

from TestHarness.utils.monitor_processes import (
    _parse_kb_fields,
    _rollup_memory,
    get_process_memory,
)

# All of this file's parsing targets /proc/<pid>/{smaps_rollup,status}, which
# only exist on Linux; skip entirely elsewhere (e.g. macOS, where TestHarness's
# own python tests also run).
LINUX_ONLY = unittest.skipUnless(
    sys.platform.startswith("linux"), "requires Linux (/proc parsing)"
)

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


@LINUX_ONLY
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


@LINUX_ONLY
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


@LINUX_ONLY
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


if __name__ == "__main__":
    unittest.main()
