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
mug = chigger.exodus.ExodusResult(reader, variable='phi', viewport=[0,0,0.5,1])
cbar = chigger.exodus.ExodusColorBar(mug)
mug.update()

sample = chigger.exodus.ExodusResultLineSampler(mug, resolution=200)
sample.update()
x = sample[0].getDistance()
y = sample[0].getSample('phi')

line = chigger.graphs.Line(x, y, width=4, label='probe')
graph = chigger.graphs.Graph(line, yaxis={'lim':[0,1]}, xaxis={'lim':[0,0.1]}, viewport=[0.5,0,1,1])

window = chigger.RenderWindow(mug, cbar, sample, graph, size=[600, 200], test=True)
window.write('line_sample_default.png')
window.start()
