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

k = chigger.graphs.Line([1,2,3,4], [1,2.75,3.5,4], corner='left-bottom', label='k', color=[1,1,1])
u = chigger.graphs.Line([1,2,3,4], [20,14,11,10], corner='right-bottom', label='u', color=[228,26,28])

graph = chigger.graphs.Graph(u, k, legend={'visible':False})
graph.setOptions('xaxis', title='X-Axis', lim=[1,4])
graph.setOptions('yaxis', title='y-Axis', lim=[1,4])
graph.setOptions('y2axis', title='y2-Axis', lim=[10,20])

window = chigger.RenderWindow(graph, size=[300,300], test=True)
window.write('dual.png')
window.start()
