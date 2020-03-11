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
camera.SetViewUp(0.6476, 0.5077, 0.5682)
camera.SetPosition(-5.0525, 0.4215, 8.0440)
camera.SetFocalPoint(0.8310, 1.0161, 0.8062)

clip0 = chigger.filters.PlaneClipper()
clip1 = chigger.filters.PlaneClipper(normal=[0,1,0])

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, camera=camera, variable='diffused', cmap='viridis',
                                  filters=[clip0, clip1])

window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('dual.png')
window.start()
