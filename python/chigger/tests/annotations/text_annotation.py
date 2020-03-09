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
text = chigger.annotations.TextAnnotation(text='This is a test.', font_size=14, text_color=[1,0,1], text_opacity=0.5)
window = chigger.RenderWindow(text, size=[300,300], test=True)
window.write('text_annotation.png')
window.start()
