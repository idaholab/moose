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

line0 = chigger.geometric.LineSource(color=[0.5,0.5,0.5])
line1 = chigger.geometric.LineSource(point1=[1,0,0], point2=[0,1,0], color=[1,0.5,0.5])

result = chigger.base.ChiggerResult(line0, line1)

window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('line_source.png')
window.start()
