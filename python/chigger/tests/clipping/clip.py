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

import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(0.4606, 0.6561, 0.5978)
camera.SetPosition(-5.8760, -5.4091, 11.0314)
camera.SetFocalPoint(0.6561, -0.1441, 0.2210)

clip = chigger.filters.PlaneClipper()

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, camera=camera, cmap='viridis', filters=[clip])

window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('clip.png')
window.start()
