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

# Camera settings so you can see the cut plane
camera = vtk.vtkCamera()
camera.SetViewUp(-0.0433, -0.8434, -0.5355)
camera.SetPosition(-5.6837, -6.0730, 10.9507)
camera.SetFocalPoint(0.4270, 0.2643, 0.4744)

clip = chigger.filters.BoxClipper(upper=[1.1, 1.1, 1.1])

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, camera=camera, cmap='viridis', filters=[clip])

window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('box_clip.png')
window.start()
