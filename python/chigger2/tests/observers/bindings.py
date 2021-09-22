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

class TestMainObserver(chigger.observers.TestObserver):

    def onTimer(self, obj, event):
        self.pressKey('r')
        window.write('bindings_select.png')
        self.pressKey('d')
        self.moveMouse(0.1, 0.1)
        window.write('bindings_move.png')
        for i in range(10):
            self.pressKey('f')
        window.write('bindings_font.png')
        for i in range(80):
            self.pressKey('a', shift=True)
        window.write('bindings_alpha.png')

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
text = chigger.annotations.TextAnnotation(text='Text', font_size=32, text_color=(1,0,1))
window = chigger.RenderWindow(text, size=(300,300), observers=[TestMainObserver(terminate=True)])
window.start()
