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

# Open the result
file_name = '../input/mug_blocks_out.e'
reader = chigger.exodus.ExodusReader(file_name)
mug = chigger.exodus.ExodusResult(reader, variable='diffused')

# Create annotation
reader.update()
times = reader.getTimes()
text = chigger.annotations.TimeAnnotation(layer=2, time=times[-1], font_size=24, text_color=[1,1,1],
                                          suffix='(h:m:s)', timedelta=True, position=[0.5, 0.5],
                                          justification='center')

# Create the window
window = chigger.RenderWindow(text, mug, size=[300,300], test=True)
window.write('time_annotation.png')
window.start()
