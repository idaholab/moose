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

class TestMakeHistogram(unittest.TestCase):
    """
    Test use of make_histogram.py for creating histograms from stochastic data.
    """

    def setUp(self):
        self._command = os.path.abspath('../make_histogram.py')
        self._jsonfile = os.path.abspath('../../examples/parameter_study/gold/main_out.json')
        self._csvfile = '../../test/tests/vectorpostprocessors/stochastic_results/gold/distributed_out_storage_0002.csv.*'
        self._imagefile = os.path.abspath('test.png')

    def tearDown(self):
        pass

    def testJSON(self):
        cmd = ['python', self._command, self._jsonfile, '--output', self._imagefile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

    def testCSV(self):
        cmd = ['python', self._command, self._csvfile, '--output', self._imagefile]
        subprocess.run(cmd)
        self.assertTrue(os.path.exists(self._imagefile))
        os.remove(self._imagefile)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
