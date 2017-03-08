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

line0 = chigger.geometric.LineSource(color=[0.5,0.5,0.5])
line1 = chigger.geometric.LineSource(point1=[1,0,0], point2=[0,1,0], color=[1,0.5,0.5])

result = chigger.base.ChiggerResult(line0, line1)

window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('line_source.png')
window.start()
