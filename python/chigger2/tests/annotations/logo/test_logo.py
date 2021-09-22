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

class TestLogo(chigger.base.ChiggerTestCase):
    def setUp(self):
        self._window = chigger.Window(size=(300,300))
        self._viewport = chigger.Viewport()
        self._test = chigger.observers.TestObserver()
        self._logo_args = dict(position=(0.5, 0.5), width=0.67, halign='center', valign='center')

    def testMoose(self):
        chigger.annotations.Logo(logo='moose', **self._logo_args)
        self.assertImage('moose.png')

    def testChiggerWhite(self):
        chigger.annotations.Logo(logo='chigger', **self._logo_args)
        self.assertImage('chigger_white.png')

    def testChiggerBlack(self):
        self._window.setParams('background', color=chigger.utils.Color(1,1,1))
        chigger.annotations.Logo(logo='chigger', **self._logo_args)
        self.assertImage('chigger_black.png')

    def testInlWhite(self):
        chigger.annotations.Logo(logo='inl', **self._logo_args)
        self.assertImage('inl_white.png')

    def testInlBlack(self):
        self._window.setParams('background', color=chigger.utils.Color(1,1,1))
        chigger.annotations.Logo(logo='inl', **self._logo_args)
        self.assertImage('inl_black.png')






if __name__ == '__main__':
    unittest.main(verbosity=2)
