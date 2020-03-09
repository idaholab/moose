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

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e', timestep=0)
mug = chigger.exodus.ExodusResult(reader, variable='phi', cmap='viridis', range=[0, 1])
cbar = chigger.exodus.ExodusColorBar(mug)
window = chigger.RenderWindow(mug, cbar, size=[600,600], test=True)

reader.update()
times = reader.getTimes()
for i in range(len(times)):
    reader.setOptions(timestep=i)
    window.write('adapt_' + str(i) + '.png')

window.start()
