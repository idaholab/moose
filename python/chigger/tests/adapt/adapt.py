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

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e', timestep=0)
mug = chigger.exodus.ExodusResult(reader, variable='phi', cmap='viridis', range=[0, 1])
cbar = chigger.exodus.ExodusColorBar(mug)
window = chigger.RenderWindow(mug, cbar, size=[600,600], test=True)

reader.update()
times = reader.getTimes()
for i in range(len(times)):
    reader.setOptions(timestep=i)
    window.write('adapt_' + str(i) + '.png')

window.start()
