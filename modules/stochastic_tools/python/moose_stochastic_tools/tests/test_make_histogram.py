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
import sys
import importlib.util
import unittest
from unittest import mock
import plotly.graph_objects as go

if importlib.util.find_spec("moose_stochastic_tools") is None:
    _stm_python_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "python")
    )
    sys.path.append(_stm_python_path)

from moose_stochastic_tools import make_histogram, MakeHistogramOptions


class TestMakeHistogram(unittest.TestCase):
    """
    Test use of make_histogram.py for creating histograms from stochastic data.
    """

    def setUp(self):
        this_dir = os.path.dirname(__file__)
        stm_dir = os.path.abspath(os.path.join(this_dir, "..", "..", ".."))
        self._jsonfile = os.path.join(
            stm_dir, "examples/parameter_study/gold/main_out.json"
        )
        self._csvfile = os.path.join(
            stm_dir,
            "test/tests/vectorpostprocessors/stochastic_results/gold/distributed_out_storage_0002.csv.*",
        )
        self._imagefile = os.path.join(this_dir, "test.png")

        self.patcher = mock.patch("plotly.io.write_image")
        self.mock_image = self.patcher.start()

    def tearDown(self):
        self.patcher.stop()

    def testJSON(self):
        opt = MakeHistogramOptions(filenames=[self._jsonfile], output=self._imagefile)
        make_histogram(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)

    def testCSV(self):
        opt = MakeHistogramOptions(filenames=[self._csvfile], output=self._imagefile)
        make_histogram(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)


if __name__ == "__main__":
    unittest.main(module=__name__, verbosity=2, buffer=True)
