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
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')

mug0 = chigger.exodus.ExodusResult(reader, variable='convected', viewport=[0,0,0.4,1], cmap='cool')
mug1 = chigger.exodus.ExodusResult(reader, variable='diffused', viewport=[0.6,0,1,1], cmap='cool')

cbar = chigger.exodus.ExodusColorBar(mug0, mug1, colorbar_origin=[0.4, 0.5, 0.0], width=0.075, length=0.2, location='top', viewport=[0,0,1,1])
cbar.setOptions('primary', 'secondary', num_ticks=6)

window = chigger.RenderWindow(mug0, mug1, cbar, size=[600,300], test=True)
window.write('common_colorbar_horiz.png')
window.start()
