#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import importlib.util
import unittest
from unittest import mock
import plotly.graph_objects as go

if importlib.util.find_spec('moose_stochastic_tools') is None:
    _stm_python_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'python'))
    sys.path.append(_stm_python_path)

from moose_stochastic_tools import visualize_sobol, VisualizeSobolOptions

class TestVisualizeSobol(unittest.TestCase):
    """
    Test use of visualize_sobol.py for processing SobolReporter output.
    """

    def setUp(self):
        this_dir = os.path.dirname(__file__)
        stm_dir = os.path.abspath(os.path.join(this_dir, '..', '..', '..'))
        self._file = os.path.join(stm_dir, 'examples/sobol/gold/main_out.json')
        self._textfile = os.path.join(this_dir, 'test.txt')
        self._imagefile = os.path.join(this_dir, 'test.png')

        self._values = ['results_results:T_avg:value', 'results_results:q_left:value']
        self._names = '{"results_results:T_avg:value":"$T_{avg}$","results_results:q_left:value":"$q_{left}$"}'
        self._param_names = ['$\\gamma$', '$q_0$', '$T_0$', '$s$']
        self._number_format = '.3g'

        self.patcher = mock.patch('plotly.io.write_image')
        self.mock_image = self.patcher.start()

    def tearDown(self):
        self.patcher.stop()

    def makeOptions(self, *args, **kwargs):
        return VisualizeSobolOptions(filename=self._file,
                                     names=self._names,
                                     param_names=self._param_names,
                                     number_format=self._number_format,
                                     *args, **kwargs)

    def testTotalTable(self):
        opt = self.makeOptions(format=1, output=self._textfile)
        expect_out =  '| $S_T$ (5.0%, 95.0%) CI   | $\\gamma$             | $q_0$               | $T_0$              | $s$                 |\n'
        expect_out += '|:-------------------------|:---------------------|:--------------------|:-------------------|:--------------------|\n'
        expect_out += '| $T_{avg}$                | 0.651 (-0.196, 2.21) | 0.126 (-1.96, 4.13) | 0.147 (-2.1, 4.54) | 0.308 (-1.25, 3.44) |\n'
        expect_out += '| $q_{left}$               | 0.813 (0.141, 1.62)  | 0.249 (-2.99, 4.33) | 0.378 (-2.8, 4.31) | 0.223 (-3.1, 4.38)  |\n'

        visualize_sobol(opt)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    def testSecondOrderTable(self):
        opt = self.makeOptions(format=1, stat='second_order', values=[self._values[0]], output=self._textfile)
        expect_out =  '| $S_{i,j}$ (5.0%, 95.0%) CI   | $\\gamma$            | $q_0$                  | $T_0$                 | $s$                   |\n'
        expect_out += '|:-----------------------------|:--------------------|:-----------------------|:----------------------|:----------------------|\n'
        expect_out += '| $\\gamma$                     | 0.726 (-1.81, 2.09) | -                      | -                     | -                     |\n'
        expect_out += '| $q_0$                        | 5.26 (-5.01, 4.36)  | 0.016 (-0.0438, 0.05)  | -                     | -                     |\n'
        expect_out += '| $T_0$                        | 3.18 (-6.16, 5.34)  | 0.131 (-0.644, 0.547)  | 0.122 (-0.414, 0.399) | -                     |\n'
        expect_out += '| $s$                          | 5.75 (-6.55, 5.79)  | 0.000477 (-1.18, 1.06) | -0.216 (-1.76, 2.22)  | 0.286 (-0.713, 0.848) |\n'
        visualize_sobol(opt)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

        opt = self.makeOptions(format=1, stat='second_order', values=[self._values[1]], output=self._textfile)
        expect_out =  '| $S_{i,j}$ (5.0%, 95.0%) CI   | $\\gamma$           | $q_0$                     | $T_0$               | $s$                  |\n'
        expect_out += '|:-----------------------------|:-------------------|:--------------------------|:--------------------|:---------------------|\n'
        expect_out += '| $\\gamma$                     | 1.12 (-3.31, 3.79) | -                         | -                   | -                    |\n'
        expect_out += '| $q_0$                        | 5.4 (-9.01, 9.68)  | -0.00704 (-0.109, 0.0949) | -                   | -                    |\n'
        expect_out += '| $T_0$                        | 9.96 (-13.3, 13.1) | 0.156 (-2.56, 1.86)       | 0.33 (-1.4, 1.4)    | -                    |\n'
        expect_out += '| $s$                          | 11.7 (-9.95, 9.15) | 0.00549 (-0.227, 0.184)   | -0.247 (-1.8, 2.17) | 0.0208 (-0.1, 0.122) |\n'
        visualize_sobol(opt)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    def testBarPlot(self):
        opt = self.makeOptions(format=3, log_scale=True, output=self._imagefile)
        visualize_sobol(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)

    def testHeatmap(self):
        opt = self.makeOptions(format=4, log_scale=True, stat="second_order", output=self._imagefile)
        visualize_sobol(opt)
        args, _ = self.mock_image.call_args
        self.assertIsInstance(args[0], go.Figure)
        self.assertEqual(args[1], self._imagefile)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
