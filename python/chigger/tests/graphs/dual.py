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

k = chigger.graphs.Line([1,2,3,4], [1,2.75,3.5,4], corner='left-bottom', label='k', color=[1,1,1])
u = chigger.graphs.Line([1,2,3,4], [20,14,11,10], corner='right-bottom', label='u', color=[228,26,28])

graph = chigger.graphs.Graph(u, k, legend={'visible':False})
graph.setOptions('xaxis', title='X-Axis', lim=[1,4])
graph.setOptions('yaxis', title='y-Axis', lim=[1,4])
graph.setOptions('y2axis', title='y2-Axis', lim=[10,20])

window = chigger.RenderWindow(graph, size=[300,300], test=True)
window.write('dual.png')
window.start()
