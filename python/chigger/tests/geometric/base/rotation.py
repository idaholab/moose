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
camera.SetViewUp(-0.2488, 0.8185, -0.5178)
camera.SetPosition(1.8403, 2.7164, 3.4098)
camera.SetFocalPoint(0.0000, 0.0000, 0.0000)

cube0 = chigger.geometric.CubeSource(position=[0,0,0], lengths=[1,1,1], rotation=[45,0,0], color=[0.5,0,0], edges=False)
cube1 = chigger.geometric.CubeSource(position=[0,0,0], lengths=[1,1,1], rotation=[0,45,0], color=[0,0.5,0], edges=False)
cube2 = chigger.geometric.CubeSource(position=[0,0,0], lengths=[1,1,1], rotation=[0,0,45], color=[0,0,0.5], edges=False)

cubes = chigger.base.ChiggerResult(cube0, cube1, cube2, camera=camera)

window = chigger.RenderWindow(cubes, size=[300,300], test=True)
window.write('rotation.png')
window.start()
