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

transform = chigger.filters.TransformFilter(scale=[3,1,1])

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='phi', cmap='viridis', filters=[transform])

window = chigger.RenderWindow(mug, size=[300,300], style='interactive2D', test=True)

window.write('scale_2d.png')
window.start()
