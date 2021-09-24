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

class TestVisualizeStatistics(unittest.TestCase):
    """
    Test use of visualize_statistics.py for processing StatisticsReporter output.
    """

    def setUp(self):
        self._command = os.path.abspath('../visualize_statistics.py')

        self._file = os.path.abspath('../../examples/parameter_study/gold/main_out.json')
        self._timefile = os.path.abspath('../../examples/parameter_study/gold/main_time_out.json')
        self._vecfile = os.path.abspath('../../examples/parameter_study/gold/main_vector_out.json')

        self._textfile = os.path.abspath('test.txt')
        self._imagefile = os.path.abspath('test.png')

        self._names = '{"results_results:T_avg:value":"$T_{avg}$","results_results:q_left:value":"$q_{left}$"}'
        self._timenames = '{"results_results:T_avg:value":"Average Temperature","results_results:q_left:value":"Flux"}'
        self._vecnames = '{"results_results:acc:T_avg:value":"Average Temperature","results_results:acc:q_left:value":"Flux"}'

        self._stat_names = '{"MEAN":"Mean","STDDEV":"Standard Deviation"}'

    def tearDown(self):
        pass

    def testMarkdownTable(self):
        cmd = ['python', self._command, self._file, '--markdown-table', '--names', self._names, '--stat-names', self._stat_names]
        expect_out =  '| Values                      | Mean                 | Standard Deviation   |\n'
        expect_out += '|:----------------------------|:---------------------|:---------------------|\n'
        expect_out += '| $T_{avg}$ (5.0%, 95.0%) CI  | 199.4 (172.4, 227.5) | 55.53 (35.73, 64.78) |\n'
        expect_out += '| $q_{left}$ (5.0%, 95.0%) CI | 179.6 (139, 223.4)   | 84.75 (54.29, 98.01) |\n'

        out = subprocess.check_output(cmd)
        self.assertEqual(out.decode(), expect_out)

        cmd.extend(['-o', self._textfile])
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    def testBarPlot(self):
        cmd = ['python', self._command, self._file, '--bar-plot', '--names', self._names, '--stat-names', self._stat_names, '-o', self._imagefile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

    def testTimeTable(self):
        cmd = ['python', self._command, self._timefile, '--markdown-table', '--names', self._timenames, '--stat-names', self._stat_names]
        cmd.extend(['--values', 'results_results:T_avg:value', 'results_results:q_left:value'])
        expect_out =  '| Values                               |   Time | Mean                 | Standard Deviation   |\n'
        expect_out += '|:-------------------------------------|-------:|:---------------------|:---------------------|\n'
        expect_out += '| Average Temperature (5.0%, 95.0%) CI |   0.25 | 227.6 (211.6, 243.5) | 32.22 (21.51, 37.73) |\n'
        expect_out += '|                                      |   0.5  | 209.7 (187.3, 232.4) | 45.38 (29.94, 52.85) |\n'
        expect_out += '|                                      |   0.75 | 202.5 (177.1, 228.8) | 52.04 (33.9, 60.63)  |\n'
        expect_out += '|                                      |   1    | 199.4 (172.4, 227.5) | 55.53 (35.73, 64.78) |\n'
        expect_out += '| Flux (5.0%, 95.0%) CI                |   0.25 | 334.4 (297.4, 372.4) | 75.04 (52.52, 86.15) |\n'
        expect_out += '|                                      |   0.5  | 213.6 (179.9, 249.1) | 69.39 (47.66, 79.06) |\n'
        expect_out += '|                                      |   0.75 | 188.3 (150.9, 228.5) | 78.01 (51.49, 89.56) |\n'
        expect_out += '|                                      |   1    | 179.6 (139, 223.4)   | 84.75 (54.29, 98.01) |\n'

        out = subprocess.check_output(cmd)
        self.assertEqual(out.decode(), expect_out)

        cmd.extend(['-o', self._textfile])
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._textfile))
        with open(self._textfile) as fid:
            out = fid.read()
        self.assertEqual(out, expect_out[:-1])
        os.remove(self._textfile)

    def testTimeTimeLine(self):
        cmd = ['python', self._command, self._timefile, '--line-plot', '--names', self._timenames, '--stat-names', self._stat_names]
        cmd.extend(['--values', 'results_results:T_avg:value', 'results_results:q_left:value'])
        cmd.extend(['-o', self._imagefile])
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

    def testTimeLine(self):
        cmd = ['python', self._command, self._timefile, '--line-plot', '--stat-names', self._stat_names]
        cmd.extend(['--values', 'results_results:T_vec:T'])
        cmd.extend(['--names', '{"results_results:T_vec:T":"Temperature"}'])
        cmd.extend(['--xvalue', 'x'])
        cmd.extend(['-o', self._imagefile])
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

    def testVectorLine(self):
        cmd = ['python', self._command, self._vecfile, '--line-plot', '--names', self._vecnames, '--stat-names', self._stat_names]
        cmd.extend(['--xvalue', 'results_results:acc:T_avg:value'])
        cmd.extend(['-o', self._imagefile])
        cmd1 = cmd
        cmd2 = cmd

        cmd1.extend(['--stats', 'MEAN'])
        subprocess.run(cmd1)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

        cmd2.extend(['--stats', 'STDDEV'])
        subprocess.run(cmd2)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
