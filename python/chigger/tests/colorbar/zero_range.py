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
reader = chigger.exodus.ExodusReader('../input/zero_variable.e')
mug = chigger.exodus.ExodusResult(reader, cmap='viridis', variable='zero')
cbar = chigger.exodus.ExodusColorBar(mug)
window = chigger.RenderWindow(mug, cbar, size=[600,400], test=True)
window.write('zero_range.png')
window.start()
