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

class MyMouseObserver(chigger.observers.MouseObserver):
    def onMouseMove(self, *args):
        print 'the mouse moved'

class TestObserver(chigger.observers.TestObserver):
    def onTimer(self, obj, event):
        self.pressKey('r')
        self.moveMouse(1,1)

text = chigger.annotations.TextAnnotation(text='test')
window = chigger.RenderWindow(text, observers=[MyMouseObserver(), TestObserver(terminate=True)])
window.start()
