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
reader = chigger.exodus.ExodusReader('../../input/variable_range.e')
result = chigger.exodus.ExodusResult(reader, representation='wireframe')
cell_result = chigger.exodus.LabelExodusResult(result, label_type='point', text_color=[0,0,1], font_size=12, justification='left', vertical_justification='top')
window = chigger.RenderWindow(result, cell_result, size=[300,300], test=True)
window.write('points.png')
window.start()
