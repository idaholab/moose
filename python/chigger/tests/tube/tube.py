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

reader = chigger.exodus.ExodusReader('../input/pipe.e')

tube = chigger.filters.TubeFilter()
pipe = chigger.exodus.ExodusResult(reader, variable='u', cmap='viridis', filters=[tube])

window = chigger.RenderWindow(pipe, size=[300,300], test=True)
window.write('tube.png')
window.start()
