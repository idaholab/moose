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
camera.SetViewUp(0.3087, 0.9262, -0.2164)
camera.SetPosition(-5.6696, 4.7995, 8.8168)
camera.SetFocalPoint(1.4721, 0.5986, 1.0268)

cube0 = chigger.geometric.CubeSource(position=[0,0,0], lengths=[1,1,1], scale=1, color=[0.5,0,0], edges=False)
cube1 = chigger.geometric.CubeSource(position=[1.5,0,0], lengths=[1,1,1], scale=2, color=[0,0.5,0], edges=False)
cube2 = chigger.geometric.CubeSource(position=[4,0,0], lengths=[1,1,1], scale=3, color=[0,0,0.5], edges=False)

cubes = chigger.base.ChiggerResult(cube0, cube1, cube2, camera=camera)

window = chigger.RenderWindow(cubes, size=[300,300], test=True)
window.write('scale.png')
window.start()
