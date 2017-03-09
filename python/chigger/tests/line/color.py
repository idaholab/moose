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

# Create a graph
graph = chigger.graphs.Graph(xaxis={'lim':[0,10], 'num_ticks':3, 'title':'x'},
                             yaxis={'lim':[0,30], 'num_ticks':5, 'title':'y'},
                             color_scheme='citrus', legend={'visible':False})

# Generate data
x = range(0,10)
y0 = range(0,10)
y1 = range(0,20,2)
y2 = range(0,30,3)

# Create line objects
line0 = chigger.graphs.Line(x, y0, label='y0')
line1 = chigger.graphs.Line(x, y1, label='y1')
line2 = chigger.graphs.Line(x, y2, label='y2')

# Add lines to graph
graph.setOptions(lines=[line0, line1, line2])

# Window
window = chigger.RenderWindow(graph, size=[500, 250], test=True)
window.write('color.png')
window.start()
