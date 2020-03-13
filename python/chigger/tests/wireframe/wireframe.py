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
camera.SetViewUp(-0.0112, -0.0149, 0.9998)
camera.SetPosition(-12.5229, -9.3291, -0.1552)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', representation='wireframe', camera=camera, colorbar={'visible':False}, cmap='viridis')
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('wireframe.png')
window.start()
