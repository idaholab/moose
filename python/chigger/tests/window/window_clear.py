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

reader = chigger.exodus.ExodusReader("../input/mug_blocks_out.e")
mug = chigger.exodus.ExodusResult(
    reader, variable="diffused", cmap="viridis", colorbar={"visible": False}
)
window = chigger.RenderWindow(mug, size=[300, 300], test=True)
window.update()

reader = chigger.exodus.ExodusReader("../input/step10_micro_out.e")
mug = chigger.exodus.ExodusResult(
    reader, variable="phi", cmap="viridis", colorbar={"visible": False}
)

window.clear()
window.append(mug)

window.start()

if window.getActive() != mug:
    raise Exception("Setting the active result is not working!")

window.write("window_clear.png")
