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

from moose_stochastic_tools import visualize_statistics, VisualizeStatisticsOptions


class TestVisualizeStatistics(unittest.TestCase):
    """
    Test use of visualize_statistics.py for processing StatisticsReporter output.
    """

    def setUp(self):
        this_dir = os.path.dirname(__file__)
        stm_dir = os.path.abspath(os.path.join(this_dir, "..", "..", ".."))

        self._file = os.path.join(
            stm_dir, "examples/parameter_study/gold/main_out.json"
        )
        self._timefile = os.path.join(
            stm_dir, "examples/parameter_study/gold/main_time_out.json"
        )
        self._vecfile = os.path.join(
            stm_dir, "examples/parameter_study/gold/main_vector_out.json"
        )

        self._textfile = os.path.join(this_dir, "test.txt")
        self._imagefile = os.path.join(this_dir, "test.png")

        self._names = '{"results_results:T_avg:value":"$T_{avg}$","results_results:q_left:value":"$q_{left}$"}'
        self._timenames = '{"results_results:T_avg:value":"Average Temperature","results_results:q_left:value":"Flux"}'
        self._vecnames = '{"results_results:acc:T_avg:value":"Average Temperature","results_results:acc:q_left:value":"Flux"}'

        self._stat_names = '{"MEAN":"Mean","STDDEV":"Standard Deviation"}'

        self.patcher = mock.patch("plotly.io.write_image")
        self.mock_image = self.patcher.start()

    def tearDown(self):
        self.patcher.stop()

    def testMarkdownTable(self):
        opt = VisualizeStatisticsOptions(
            filenames=[self._file],
            format=1,
            names=self._names,
            stat_names=self._stat_names,
            output=self._textfile,
        )
        expect_out = "| Values                      | Mean                 | Standard Deviation   |\n"
        expect_out += "|:----------------------------|:---------------------|:---------------------|\n"
        expect_out += "| $T_{avg}$ (5.0%, 95.0%) CI  | 199.4 (172.4, 227.5) | 55.53 (35.73, 64.78) |\n"
        expect_out += "| $q_{left}$ (5.0%, 95.0%) CI | 179.6 (139, 223.4)   | 84.75 (54.29, 98.01) |\n"

        visualize_statistics(opt)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    # @unittest.skipIf(find_spec('kaleido') is None, "Kaleido must be installed.")
    def testBarPlot(self):
        opt = VisualizeStatisticsOptions(
            filenames=[self._file],
            format=3,
            names=self._names,
            stat_names=self._stat_names,
            output=self._imagefile,
        )
        visualize_statistics(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)

    def testTimeTable(self):
        opt = VisualizeStatisticsOptions(
            filenames=[self._timefile],
            format=1,
            names=self._timenames,
            stat_names=self._stat_names,
            values=["results_results:T_avg:value", "results_results:q_left:value"],
            output=self._textfile,
        )
        expect_out = "| Values                               |   Time | Mean                 | Standard Deviation   |\n"
        expect_out += "|:-------------------------------------|-------:|:---------------------|:---------------------|\n"
        expect_out += "| Average Temperature (5.0%, 95.0%) CI |   0.25 | 227.6 (211.6, 243.5) | 32.22 (21.51, 37.73) |\n"
        expect_out += "|                                      |   0.5  | 209.7 (187.3, 232.4) | 45.38 (29.94, 52.85) |\n"
        expect_out += "|                                      |   0.75 | 202.5 (177.1, 228.8) | 52.04 (33.9, 60.63)  |\n"
        expect_out += "|                                      |   1    | 199.4 (172.4, 227.5) | 55.53 (35.73, 64.78) |\n"
        expect_out += "| Flux (5.0%, 95.0%) CI                |   0.25 | 334.4 (297.4, 372.4) | 75.04 (52.52, 86.15) |\n"
        expect_out += "|                                      |   0.5  | 213.6 (179.9, 249.1) | 69.39 (47.66, 79.06) |\n"
        expect_out += "|                                      |   0.75 | 188.3 (150.9, 228.5) | 78.01 (51.49, 89.56) |\n"
        expect_out += "|                                      |   1    | 179.6 (139, 223.4)   | 84.75 (54.29, 98.01) |\n"

        visualize_statistics(opt)

        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    def testTimeTimeLine(self):
        opt = VisualizeStatisticsOptions(
            filenames=[self._timefile],
            format=4,
            names=self._timenames,
            stat_names=self._stat_names,
            values=["results_results:T_avg:value", "results_results:q_left:value"],
            output=self._imagefile,
        )
        visualize_statistics(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)

    def testTimeLine(self):
        opt = VisualizeStatisticsOptions(
            filenames=[self._timefile],
            format=4,
            names={"results_results:T_vec:T": "Temperature"},
            stat_names=self._stat_names,
            values=["results_results:T_vec:T"],
            xvalue="x",
            output=self._imagefile,
        )
        visualize_statistics(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)

    def testVectorLine(self):
        for stat in ["MEAN", "STDDEV"]:
            opt = VisualizeStatisticsOptions(
                filenames=[self._vecfile],
                format=4,
                names=self._vecnames,
                stat_names=self._stat_names,
                xvalue="results_results:acc:T_avg:value",
                stats=[stat],
                output=self._imagefile,
            )
            visualize_statistics(opt)
            args, _ = self.mock_image.call_args
            self.assertIsInstance(args[0], go.Figure)
            self.assertEqual(args[1], self._imagefile)


if __name__ == "__main__":
    unittest.main(module=__name__, verbosity=2, buffer=True)
