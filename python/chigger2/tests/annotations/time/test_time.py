#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from moosetools import chigger

class TestTime(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._time = chigger.annotations.Time(days=1000)

    def testDefault(self):
        self.assertImage('default.png')

    def testChange(self):
        self.setObjectParams(self._time, days=1980, hours=10, minutes=20, seconds=30, milliseconds=400, microseconds=500)
        self.assertImage('change.png')

if __name__ == '__main__':
    import unittest
    unittest.main(verbosity=2)
