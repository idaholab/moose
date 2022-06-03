#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import numpy as np
import unittest

class TestMultiLevel(unittest.TestCase):
    def test(self):
        data = np.genfromtxt('parent_out_sub10_sub20.csv', dtype=float, delimiter=',', names=True)

        # We should have two time steps plus the initial state
        self.assertEqual(len(data), 3)

        # parent, sub, and sub-sub times should all be equivalent
        for i in range(len(data)):
            self.assertEqual(data['sub_time'][i], data['parent_time'][i])
            self.assertEqual(data['sub_time'][i], data['time'][i])

        # parent, sub, and sub-sub dts should all be equivalent
        for i in range(len(data['sub_dt'])):
            self.assertEqual(data['sub_dt'][i], data['dt'][i])
            self.assertEqual(data['sub_dt'][i], data['parent_dt'][i])

        # The second timestep definitely shouldn't be equal to dt
        self.assertNotEqual(data['time'][2], data['dt'][2])

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
