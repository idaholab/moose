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
mug = chigger.exodus.ExodusResult(reader, variable='diffused', cmap='viridis', colorbar={'visible':False})
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.update()

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='phi', cmap='viridis', colorbar={'visible':False})

window.clear()
window.append(mug)

window.start()

if window.getActive() != mug:
    raise Exception('Setting the active result is not working!')

window.write('window_clear.png')
