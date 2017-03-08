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
