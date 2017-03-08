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

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e')
mug = chigger.exodus.ExodusResult(reader, viewport=[0, 0, 0.25, 1], variable='phi')
mug.update()
sample = chigger.exodus.ExodusResultLineSampler(mug, point1=(0, 0.05, 0), point2=(0.1, 0.05, 0), resolution=200)
sample.update()
x = sample[0].getDistance()
y = sample[0].getSample('invalid_name') # testing this error
