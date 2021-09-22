#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import unittest
from moosetools import chigger

class TestAdapt(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._reader = chigger.exodus.ExodusReader(filename='../input/step10_micro_out.e', timestep=0)
        self._mug = chigger.exodus.ExodusSource(variable='phi', lim=(0, 1))

    def testDefault(self):
        for i in [0,4,9]:
            filename = 'adapt_' + str(i) + '.png'
            self.setObjectParams(self._reader, timestep=i)
            self.assertImage(filename, threshold=10)

if __name__ == '__main__':
    unittest.main(verbosity=2)
