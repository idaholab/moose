#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import chigger

class MyKeyObserver(chigger.observers.KeyObserver):
    def onKeyPress(self, key, *args):
        if key == 'z':
            print 'the key was pressed'

class TestObserver(chigger.observers.TestObserver):
    def onTimer(self, obj, event):
        self.pressKey('z')

window = chigger.RenderWindow(observers=[MyKeyObserver(), TestObserver(terminate=True)])
window.start()
