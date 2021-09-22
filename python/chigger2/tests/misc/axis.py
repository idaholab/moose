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
from chigger import misc, observers

ax = misc.Axis2D(title='Axis Title',
                 title_position=0.75,
                 point2=(0.15, 0.25),
                 point1=(0.75, 0.75),
                 range=(1,5.3423),
                 adjust_range=True,
                 linewidth=5,
                 format='%5.5f',
                 #major_nticks=7,
                 major_offset=18,
                 minor_nticks=3,
                 color=(0,0,1))

view = chigger.Viewport(ax)
window = chigger.Window(view, size=(1200,600), background=(1,1,1))
window.write('axis.png')
window.start()
