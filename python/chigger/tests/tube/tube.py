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

reader = chigger.exodus.ExodusReader('../input/pipe.e')

tube = chigger.filters.TubeFilter()
pipe = chigger.exodus.ExodusResult(reader, variable='u', cmap='viridis', filters=[tube])

window = chigger.RenderWindow(pipe, size=[300,300], test=True)
window.write('tube.png')
window.start()
