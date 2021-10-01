#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import subprocess

class TestVisualizeSobol(unittest.TestCase):
    """
    Test use of visualize_sobol.py for processing SobolReporter output.
    """

    def setUp(self):
        self._command = os.path.abspath('../visualize_sobol.py')
        self._file = os.path.abspath('../../examples/sobol/gold/main_out.json')
        self._textfile = os.path.abspath('test.txt')
        self._imagefile = os.path.abspath('test.png')

        self._values = ['results_results:T_avg:value', 'results_results:q_left:value']
        self._names = '{"results_results:T_avg:value":"$T_{avg}$","results_results:q_left:value":"$q_{left}$"}'
        self._param_names = ['$\gamma$', '$q_0$', '$T_0$', '$s$']
        self._number_format = '.3g'

    def tearDown(self):
        pass

    def testTotalTable(self):
        cmd = ['python', self._command, self._file, '--names', self._names, '--param-names'] + self._param_names
        cmd += ['--markdown-table', '--number-format', self._number_format]
        cmd += ['--stat', 'total']
        expect_out =  '| $S_T$ (5.0%, 95.0%) CI   | $\gamma$             | $q_0$                 | $T_0$               | $s$                   |\n'
        expect_out += '|:-------------------------|:---------------------|:----------------------|:--------------------|:----------------------|\n'
        expect_out += '| $T_{avg}$                | 0.717 (0.0491, 1.86) | -0.234 (-2.24, 3.95)  | 0.0141 (-1.57, 3.4) | -0.0668 (-1.71, 3.59) |\n'
        expect_out += '| $q_{left}$               | 0.717 (-0.893, 2.51) | -0.0682 (-5.27, 7.23) | 0.282 (-3.24, 4.79) | -0.0276 (-5.11, 6.89) |\n'

        out = subprocess.check_output(cmd)
        self.assertEqual(out.decode(), expect_out)
        cmd += ['-o', self._textfile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    def testSecondOrderTable(self):
        cmd_base = ['python', self._command, self._file, '--names', self._names, '--param-names'] + self._param_names
        cmd_base += ['--markdown-table', '--number-format', self._number_format]
        cmd_base += ['--stat', 'second_order']

        cmd = cmd_base
        cmd += ['--values', self._values[0]]
        expect_out =  '| $S_{i,j}$ (5.0%, 95.0%) CI   | $\gamma$            | $q_0$                  | $T_0$                 | $s$                   |\n'
        expect_out += '|:-----------------------------|:--------------------|:-----------------------|:----------------------|:----------------------|\n'
        expect_out += '| $\gamma$                     | 0.708 (-2.78, 3.16) | -                      | -                     | -                     |\n'
        expect_out += '| $q_0$                        | 5.34 (-6.45, 6.59)  | 0.022 (-0.0924, 0.108) | -                     | -                     |\n'
        expect_out += '| $T_0$                        | 2.21 (-8.01, 8.19)  | 0.1 (-0.874, 0.918)    | 0.0663 (-0.34, 0.447) | -                     |\n'
        expect_out += '| $s$                          | 3.24 (-8.12, 8.32)  | 0.0315 (-1.13, 1.22)   | 0.0113 (-1.63, 1.86)  | 0.123 (-0.627, 0.714) |\n'
        out = subprocess.check_output(cmd)
        self.assertEqual(out.decode(), expect_out)
        cmd += ['-o', self._textfile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

        cmd = cmd_base
        cmd += ['--values', self._values[1]]
        expect_out =  '| $S_{i,j}$ (5.0%, 95.0%) CI   | $\gamma$            | $q_0$                   | $T_0$                | $s$                    |\n'
        expect_out += '|:-----------------------------|:--------------------|:------------------------|:---------------------|:-----------------------|\n'
        expect_out += '| $\gamma$                     | 0.739 (-4.09, 3.95) | -                       | -                    | -                      |\n'
        expect_out += '| $q_0$                        | 1.15 (-9.14, 9.82)  | 0.0203 (-0.166, 0.167)  | -                    | -                      |\n'
        expect_out += '| $T_0$                        | 1.34 (-11.5, 11.7)  | -0.0834 (-1.66, 2.12)   | 0.225 (-1.38, 1.33)  | -                      |\n'
        expect_out += '| $s$                          | 1.1 (-8.68, 9.75)   | -0.0442 (-0.392, 0.554) | -0.127 (-1.68, 2.12) | 0.0257 (-0.211, 0.193) |\n'
        cmd += ['-o', self._textfile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    def testBarPlot(self):
        cmd = ['python', self._command, self._file, '--names', self._names, '--param-names'] + self._param_names
        cmd += ['--bar-plot', '--log-scale']
        cmd += ['--stat', 'total']
        cmd += ['-o', self._imagefile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

    def testHeatmap(self):
        cmd = ['python', self._command, self._file, '--names', self._names, '--param-names'] + self._param_names
        cmd += ['--heatmap', '--log-scale']
        cmd += ['--stat', 'second_order']
        cmd += ['-o', self._imagefile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
