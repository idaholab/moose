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
import csv
import tempfile
import argparse
import mooseutils.csvdiff


class TestCsvDiff(unittest.TestCase):
    """
    Test csvdiff
    """
    def setUp(self):
        self._tmpfile0 = tempfile.mkstemp(text=True)[-1]
        with open(self._tmpfile0, 'w', encoding='utf-8') as fid:
            csvwriter = csv.writer(fid)
            csvwriter.writerow(['name1','name2','name3'])
            csvwriter.writerow(['1','2.22', '3.3'])

    def tearDown(self):
        os.remove(self._tmpfile0)

    def testSummary(self):
        args = mooseutils.csvdiff.parseArgs(['--summary' ,self._tmpfile0])
        with mooseutils.csvdiff.CSVSummary(args) as csv_summary:
            self.assertIn(
            "TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: 0 @ t0\n\n"
            "GLOBAL VARIABLES relative 5.5e-06 floor 1e-11\n"
            "    name1                    # min: 1.000e+00 @ t0          max: 1.000e+00 @ t0\n"
            "    name2                    # min: 2.220e+00 @ t0          max: 2.220e+00 @ t0\n"
            "    name3                    # min: 3.300e+00 @ t0          max: 3.300e+00 @ t0",
            csv_summary.summary())



if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
