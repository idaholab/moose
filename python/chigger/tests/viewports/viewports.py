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
mug0 = chigger.exodus.ExodusResult(reader, variable='convected', viewport=[0, 0, 0.5, 1], cmap='shock')
mug1 = chigger.exodus.ExodusResult(reader, variable='diffused', viewport=[0.5, 0, 1, 1], cmap='coolwarm')
window = chigger.RenderWindow(mug1, mug0, size=[600, 300], test=True)
window.write('viewports.png')
window.start()
