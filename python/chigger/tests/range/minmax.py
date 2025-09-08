#!/usr/bin/env python3
# pylint: disable=missing-docstring
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import chigger

reader = chigger.exodus.ExodusReader("../input/mug_blocks_out.e", timestep=0)
mug = chigger.exodus.ExodusResult(reader, variable="diffused", min=0.5, max=1.8)
cbar = chigger.exodus.ExodusColorBar(
    mug, primary={"precision": 2, "num_ticks": 3, "notation": "fixed"}
)
window = chigger.RenderWindow(mug, cbar, size=[300, 300], test=True)

for i in range(2):
    reader.setOptions(timestep=i)
    window.write("minmax_" + str(i) + ".png")
window.start()
