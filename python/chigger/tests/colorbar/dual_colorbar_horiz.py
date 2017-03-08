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

mug0 = chigger.exodus.ExodusResult(reader, variable='convected', viewport=[0,0,0.5,1], cmap='coolwarm')
cbar0 = chigger.exodus.ExodusColorBar(mug0, location='bottom', primary={'num_ticks':6})

mug1 = chigger.exodus.ExodusResult(reader, variable='diffused', viewport=[0.5,0,1,1], cmap='shock')
cbar1 = chigger.exodus.ExodusColorBar(mug1, location='top', primary={'num_ticks':6})

window = chigger.RenderWindow(mug0, cbar0, mug1, cbar1, size=[600,300], test=True)
window.write('dual_colorbar_horiz.png')
window.start()
