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
import sys
import unittest
import json
import io
# so we can find our libraries, no matter how we're called
findbin = os.path.dirname(os.path.realpath(sys.argv[0]))
sys.path.append(os.path.join(findbin, "../"))

from readers import reader_utils

class TestReaderUtils(unittest.TestCase):
    """
    Test that the functions in dbutils work correctly
    """

    def testfillMissingValues(self):
        """
        Test that fillValues correctly fills arrays containing missing values
        """

        missing_value = 500.0

        # Linear fit
        T = [0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0]
        data = [-10.5051, -10.5051, 500.0000, 500.0000, 500.0000, 500.0000, 500.0000, 500.0000]
        gold = [-10.5051, -10.5051, -10.5051, -10.5051, -10.5051, -10.5051, -10.5051, -10.5051]
        notegold = "Missing array values in original database have been filled using a linear fit. Original values are [-10.5051, -10.5051, 500.0, 500.0, 500.0, 500.0, 500.0, 500.0]"

        output, note = reader_utils.fillMissingValues(T, data, 'linear', missing_value)
        self.assertEqual(output, gold)
        self.assertEqual(note, notegold)

        # Fourth-order polynomial fit
        data = [-.5426, -.3614, -.1267, .1260, .4402, .7793, 500.0000, 500.0000]
        gold = [-0.5426, -0.3614, -0.1267, 0.126, 0.4402, 0.7793, 1.1718, 1.646]
        notegold = "Missing array values in original database have been filled using a fourth-order fit. Original values are [-0.5426, -0.3614, -0.1267, 0.126, 0.4402, 0.7793, 500.0, 500.0]"

        output, note = reader_utils.fillMissingValues(T, data, 'fourth-order', missing_value)
        self.assertEqual(output, gold)
        self.assertEqual(note, notegold)

        # Maier-Kelly  fit
        T = [0.01, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0]
        data = [55.5420, 50.3678, 44.4606, 39.1093, 33.8926, 29.8196, 500.0000, 500.0000]
        gold = [55.5420, 50.3678, 44.4606, 39.1093, 33.8926, 29.8196, 26.2832, 23.1649]
        notegold = "Missing array values in original database have been filled using a maier-kelly fit. Original values are [55.542, 50.3678, 44.4606, 39.1093, 33.8926, 29.8196, 500.0, 500.0]"

        output, note = reader_utils.fillMissingValues(T, data, 'maier-kelly', missing_value)
        self.assertEqual(output, gold)
        self.assertEqual(note, notegold)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
