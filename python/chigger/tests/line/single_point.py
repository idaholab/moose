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

line = chigger.graphs.Line(x=[1], y=[2], marker='circle', label='y-data')
graph = chigger.graphs.Graph(line)
graph.setOptions('xaxis', lim=[0,3])
graph.setOptions('yaxis', lim=[0,3])

window = chigger.RenderWindow(graph, size=[300,300], test=True)
window.write('single_point.png')
window.start()
