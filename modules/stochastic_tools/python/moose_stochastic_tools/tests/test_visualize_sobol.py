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

from moose_stochastic_tools import visualize_sobol, VisualizeSobolOptions
from moose_stochastic_tools.tests.test_visualize_statistics import CI_CELL

DASH_CELL = r"\s*-\s*"


class TestVisualizeSobol(unittest.TestCase):
    """
    Test use of visualize_sobol.py for processing SobolReporter output.
    """

    def setUp(self):
        this_dir = os.path.dirname(__file__)
        stm_dir = os.path.abspath(os.path.join(this_dir, "..", "..", ".."))
        self._file = os.path.join(stm_dir, "examples/sobol/gold/main_out.json")
        self._textfile = os.path.join(this_dir, "test.txt")
        self._imagefile = os.path.join(this_dir, "test.png")

        self._values = ["results_results:T_avg:value", "results_results:q_left:value"]
        self._names = '{"results_results:T_avg:value":"$T_{avg}$","results_results:q_left:value":"$q_{left}$"}'
        self._param_names = ["$\\gamma$", "$q_0$", "$T_0$", "$s$"]
        self._number_format = ".3g"

        self.patcher = mock.patch("plotly.io.write_image")
        self.mock_image = self.patcher.start()

    def tearDown(self):
        self.patcher.stop()

    def makeOptions(self, *args, **kwargs):
        return VisualizeSobolOptions(
            filename=self._file,
            names=self._names,
            param_names=self._param_names,
            number_format=self._number_format,
            *args,
            **kwargs,
        )

    def testTotalTable(self):
        opt = self.makeOptions(format=1, output=self._textfile)
        expect_out = rf"""^\| \$S_T\$ \(5\.0%, 95\.0%\) CI\s+\|\s+\$\\gamma\$\s+\|\s+\$q_0\$\s+\|\s+\$T_0\$\s+\|\s+\$s\$\s+\|
\|:?-+\|:?-+\|:?-+\|:?-+\|:?-+\|
\| \$T_{{avg}}\$ \s*\| {CI_CELL} | {CI_CELL} | {CI_CELL} | {CI_CELL} |
\| \$q_{{left}}\$ \s*\| {CI_CELL} | {CI_CELL} | {CI_CELL} | {CI_CELL} |$"""

        visualize_sobol(opt)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertRegex(out, expect_out)
        os.remove(self._textfile)

    def testSecondOrderTable(self):
        opt = self.makeOptions(
            format=1,
            stat="second_order",
            values=[self._values[0]],
            output=self._textfile,
        )
        expect_out = rf"""^\| \$S_{{i,j}}\$ \(5\.0%, 95\.0%\) CI\s+\|\s+\$\\gamma\$\s+\|\s+\$q_0\$\s+\|\s+\$T_0\$\s+\|\s+\$s\$\s+\|
\|:?-+\|:?-+\|:?-+\|:?-+\|:?-+\|
\| \$\\gamma\$ \s*\| {CI_CELL} | {DASH_CELL} \| {DASH_CELL} \| {DASH_CELL} \|
\| \$q_0\$ \s*\| {CI_CELL} | {CI_CELL} | {DASH_CELL} \| {DASH_CELL} \|
\| \$T_0\$ \s*\| {CI_CELL} | {CI_CELL} | {CI_CELL} | {DASH_CELL} \|
\| \$s\$ \s*\| {CI_CELL} | {CI_CELL} | {CI_CELL} | {CI_CELL} |$"""
        visualize_sobol(opt)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertRegex(out, expect_out)
        os.remove(self._textfile)

        opt = self.makeOptions(
            format=1,
            stat="second_order",
            values=[self._values[1]],
            output=self._textfile,
        )
        expect_out = rf"""^\| \$S_{{i,j}}\$ \(5\.0%, 95\.0%\) CI\s+\|\s+\$\\gamma\$\s+\|\s+\$q_0\$\s+\|\s+\$T_0\$\s+\|\s+\$s\$\s+\|
\|:?-+\|:?-+\|:?-+\|:?-+\|:?-+\|
\| \$\\gamma\$ \s*\| {CI_CELL} | {DASH_CELL} \| {DASH_CELL} \| {DASH_CELL} \|
\| \$q_0\$ \s*\| {CI_CELL} | {CI_CELL} | {DASH_CELL} \| {DASH_CELL} \|
\| \$T_0\$ \s*\| {CI_CELL} | {CI_CELL} | {CI_CELL} | {DASH_CELL} \|
\| \$s\$ \s*\| {CI_CELL} | {CI_CELL} | {CI_CELL} | {CI_CELL} |$"""
        visualize_sobol(opt)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertRegex(out, expect_out)
        os.remove(self._textfile)

    def testBarPlot(self):
        opt = self.makeOptions(format=3, log_scale=True, output=self._imagefile)
        visualize_sobol(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)

    def testHeatmap(self):
        opt = self.makeOptions(
            format=4, log_scale=True, stat="second_order", output=self._imagefile
        )
        visualize_sobol(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)


if __name__ == "__main__":
    unittest.main(module=__name__, verbosity=2, buffer=True)
