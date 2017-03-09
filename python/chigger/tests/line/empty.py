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

line = chigger.graphs.Line()
graph = chigger.graphs.Graph(line,
                            xaxis={'lim':[0,10], 'num_ticks':3, 'title':'x'},
                            yaxis={'lim':[0,30], 'num_ticks':5, 'title':'y'},
                            color_scheme='citrus', legend={'visible':False})

window = chigger.RenderWindow(graph, size=[500, 250], test=True)
window.write('empty.png')
window.start()
