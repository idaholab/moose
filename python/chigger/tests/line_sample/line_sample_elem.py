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

reader = chigger.exodus.ExodusReader('../input/simple_diffusion_out.e')
result = chigger.exodus.ExodusResult(reader, variable='aux', viewport=[0,0,0.5,1], opacity=0.1,
                                     range=[-1, 1])
cbar = chigger.exodus.ExodusColorBar(result)
result.update()

sample = chigger.exodus.ExodusResultLineSampler(result, point1=[0,0,0.5], resolution=100)
sample.update()

x = sample[0].getDistance()
y = sample[0].getSample('aux')

line = chigger.graphs.Line(x, y, width=4, label='probe')
graph = chigger.graphs.Graph(line, viewport=[0.5,0,1,1])
graph.setOptions('xaxis', lim=[0, 1.4])
graph.setOptions('yaxis', lim=[0, 1.])

window = chigger.RenderWindow(result, cbar, sample, graph, size=[800, 400], test=True)

window.write('line_sample_elem.png')
window.start()
