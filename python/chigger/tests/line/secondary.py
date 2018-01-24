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

n = 3
line0 = chigger.graphs.Line(marker='circle', color=[0,0,1])
line1 = chigger.graphs.Line(marker='plus', color=[0,1,0], corner='right-top')
graph = chigger.graphs.Graph(line0, line1)
graph.setOptions('xaxis', lim=[0,n])
graph.setOptions('yaxis', lim=[0,n])
graph.setOptions('x2axis', lim=[0,n])
graph.setOptions('y2axis', lim=[0,n])
graph.setOptions('legend', visible=False)

window = chigger.RenderWindow(graph, size=[300,300], test=True)
window.write('secondary_initial.png')

for i in range(n+1):
    line0.setOptions(x=[i], y=[i], append=True)
    line1.setOptions(x=[i], y=[n-i], append=True)
    window.write('secondary_' + str(i) + '.png')

window.start()
