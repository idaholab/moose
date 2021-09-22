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

class TestImage(chigger.base.ChiggerTestCase):
    def setUp(self):
        super().setUp()
        self._moose = chigger.annotations.Image(filename='../../../logos/moose.png')

    def testDefault(self):
        self.assertImage('default.png')

    def testWidth(self):
        self.setObjectParams(self._moose, width=1)
        self.assertImage('width.png')

    def testHeight(self):
        self.setObjectParams(self._moose, height=1)
        self.assertImage('height.png')

    def testHeightAndWidth(self):
        self.setObjectParams(self._moose, width=1, height=1)
        self.assertImage('height_and_width.png')

    def testPosition(self):
        self.setObjectParams(self._moose, position=(0.5,0.5))
        self.assertImage('position.png')

    def testHorizontalAlign(self):
        self.setObjectParams(self._moose, halign='center', width=1, position=(0.5,0.5))
        self.assertImage('horizontal_alignment.png')

    def testVeriticalAlign(self):
        self.setObjectParams(self._moose, valign='top', position=(0,1))
        self.assertImage('vertical_alignment.png')

    def testOpacity(self):
        self.setObjectParams(self._moose, opacity=0.2)
        self.assertImage('opacity.png')

if __name__ == '__main__':
    import unittest
    unittest.main(verbosity=2)
