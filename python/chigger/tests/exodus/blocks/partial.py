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
reader = chigger.exodus.ExodusReader('../../input/block_vars_out.e')
result = chigger.exodus.ExodusResult(reader, variable='right_elemental')
window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('partial.png')
window.start()
