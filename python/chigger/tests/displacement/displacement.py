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

reader = chigger.exodus.ExodusReader('../input/displace.e')
exodus = chigger.exodus.ExodusResult(reader, color=[0,0,1])

window = chigger.RenderWindow(exodus, size=[300,300], test=True)

reader.update()
times = reader.getTimes()
for i in range(4):
    reader.setOptions(timestep=None, time=times[i])
    window.write('displacement_' + str(i) + '.png')
window.start()
