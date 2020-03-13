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
camera.SetViewUp(0.0291, 0.1428, 0.9893)
camera.SetPosition(-3.0062, -15.1563, 2.4016)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
result = chigger.exodus.ExodusResult(reader, variable='diffused', edges=True, edge_color=[1,1,1], camera=camera, cmap='viridis')

window = chigger.RenderWindow(result, size=[300,300], test=True, antialiasing=20)
window.update(); window.resetCamera() #TODO: This is needed to make results render correctly, not sure why
window.write('mesh.png')
window.start()
