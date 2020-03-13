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
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug0 = chigger.exodus.ExodusResult(reader, variable='convected', viewport=[0,0,0.5,1], cmap='PiYG')
cbar0 = chigger.exodus.ExodusColorBar(mug0, primary={'num_ticks':6})
mug1 = chigger.exodus.ExodusResult(reader, variable='diffused', viewport=[0.5,0,1,1], cmap='BrBG')
cbar1 = chigger.exodus.ExodusColorBar(mug1, primary={'num_ticks':6})
window = chigger.RenderWindow(mug0, cbar0, mug1, cbar1, size=[600,300], test=True)
window.write('dual_colorbar.png')
window.start()
