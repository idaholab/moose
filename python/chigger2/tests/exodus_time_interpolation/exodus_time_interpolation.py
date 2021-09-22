#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import chigger

left_reader = chigger.exodus.ExodusReader('../input/simple_time.e', time=1, timestep=None)
left_result = chigger.exodus.ExodusResult(left_reader, name='left', variable='u',
                                          cmap='viridis', range=(1,3), viewport=(0,0,0.33,1))

middle_reader = chigger.exodus.ExodusReader('../input/simple_time.e', time=1.5, timestep=None)
middle_result = chigger.exodus.ExodusResult(middle_reader, name='middle', variable='u',
                                            cmap='viridis', range=(1,3), viewport=(0.33,0,0.66,1))

right_reader = chigger.exodus.ExodusReader('../input/simple_time.e', time=2, timestep=None)
right_result = chigger.exodus.ExodusResult(right_reader, name='right', variable='u',
                                           cmap='viridis', range=(1,3), viewport=(0.66,0,1,1))

window = chigger.RenderWindow(left_result, middle_result, right_result, size=(900,300), test=True)
window.write('exodus_time_interpolation.png')
window.start()
