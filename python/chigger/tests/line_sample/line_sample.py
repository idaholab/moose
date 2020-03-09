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

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='phi')
mug.update()

p0 = (0, 0.05, 0)
p1 = (0.1, 0.05, 0)
sample = chigger.exodus.ExodusResultLineSampler(mug, point1=p0, point2=p1, resolution=200)
sample.update()
x = sample[0].getDistance()
y = sample[0].getSample('phi')

print(x[98], y[98])

line = chigger.graphs.Line(x, y, width=4, label='probe')
graph = chigger.graphs.Graph(line, yaxis={'lim':[0,1]}, xaxis={'lim':[0,0.1]})

window = chigger.RenderWindow(graph, size=[600, 200], test=True)
window.write('line_sample.png')
window.start()
