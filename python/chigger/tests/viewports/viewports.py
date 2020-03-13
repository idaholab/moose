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
mug0 = chigger.exodus.ExodusResult(reader, variable='convected', viewport=[0, 0, 0.5, 1], cmap='shock')
mug1 = chigger.exodus.ExodusResult(reader, variable='diffused', viewport=[0.5, 0, 1, 1], cmap='coolwarm')
window = chigger.RenderWindow(mug1, mug0, size=[600, 300], test=True)
window.write('viewports.png')
window.start()
