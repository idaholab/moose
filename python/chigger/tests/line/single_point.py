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

line = chigger.graphs.Line(x=[1], y=[2], marker='circle', label='y-data')
graph = chigger.graphs.Graph(line)
graph.setOptions('xaxis', lim=[0,3])
graph.setOptions('yaxis', lim=[0,3])

window = chigger.RenderWindow(graph, size=[300,300], test=True)
window.write('single_point.png')
window.start()
