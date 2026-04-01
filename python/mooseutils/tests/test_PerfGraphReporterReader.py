#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest

from mooseutils.PerfGraphReporterReader import PerfGraphReporterReader


class TestPerfGraphReporterReader(unittest.TestCase):
    """Test mooseutils.PerfGraphReporterReader."""

    def test_read(self):
        """Test reading perf graph output."""
        file = os.path.abspath(
            "../../../test/tests/reporters/perf_graph_reporter/gold/perf_graph_reporter_json.json"
        )
        PerfGraphReporterReader(file)


if __name__ == "__main__":
    unittest.main(module=__name__, verbosity=2, buffer=True)
