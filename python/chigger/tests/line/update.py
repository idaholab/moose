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

n = 5
line = chigger.graphs.Line(0, 0, marker='circle')
graph = chigger.graphs.Graph(line)
graph.setOptions('xaxis', lim=[0,n])
graph.setOptions('yaxis', lim=[0,2*n])
graph.setOptions('legend', visible=False)

window = chigger.RenderWindow(graph, size=[300,300], test=True)

for i in range(n+1):
    line.setOptions(x=[i], y=[2*i])
    window.write('update_' + str(i) + '.png')

window.start()
