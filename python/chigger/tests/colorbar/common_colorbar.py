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

mug0 = chigger.exodus.ExodusResult(reader, variable='convected', viewport=[0,0,0.4,1], layer=2, cmap='coolwarm')

mug1 = chigger.exodus.ExodusResult(reader, variable='diffused', viewport=[0.6,0,1,1], layer=2, cmap='coolwarm')

cbar = chigger.exodus.ExodusColorBar(mug0, mug1, colorbar_origin=[0.50625, 0.25], viewport=[0, 0, 1, 1], width=0.025, location='left')
cbar.setOptions('primary', 'secondary', num_ticks=6)

window = chigger.RenderWindow(mug0, mug1, cbar, size=[600,300], test=True)
window.write('common_colorbar.png')
window.start()
