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

group = chigger.base.ResultGroup()
group.add(chigger.exodus.ExodusResult, reader, variable='diffused', cmap='viridis', block=['76'])
group.add(chigger.exodus.ExodusResult, reader, variable='convected', cmap='jet', block=['1'])
window = chigger.RenderWindow(group, size=[300,300], test=True)
window.write('result_group.png')
window.start()
