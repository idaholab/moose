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

group = chigger.base.ResultGroup()
group.add(chigger.exodus.ExodusResult, reader, variable='diffused', cmap='viridis', block=['76'])
group.add(chigger.exodus.ExodusResult, reader, variable='convected', cmap='jet', block=['1'])
window = chigger.RenderWindow(group, size=[300,300], test=True)
window.write('result_group.png')
window.start()
