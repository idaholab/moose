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
reader = chigger.exodus.ExodusReader('../../input/variable_range.e')
result = chigger.exodus.ExodusResult(reader, variable='u', representation='wireframe')
cell_result = chigger.exodus.LabelExodusResult(result, text_color=[0,1,1], font_size=10)
window = chigger.RenderWindow(result, cell_result, size=[800,800], test=True)
window.write('variable.png')
window.start()
