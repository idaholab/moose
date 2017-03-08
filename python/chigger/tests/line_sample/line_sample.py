#!/usr/bin/env python
#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
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

print x[98], y[98]

line = chigger.graphs.Line(x, y, width=4, label='probe')
graph = chigger.graphs.Graph(line, yaxis={'lim':[0,1]}, xaxis={'lim':[0,0.1]})

window = chigger.RenderWindow(graph, size=[600, 200], test=True)
window.write('line_sample.png')
window.start()
