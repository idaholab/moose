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
camera.SetViewUp(0.1829, 0.7889, 0.5867)
camera.SetPosition(-9.9663, -4.0748, 7.8279)
camera.SetFocalPoint(0.0000, 0.0000, -0.7582)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
contour = chigger.filters.ContourFilter(levels=[0.25, 0.5, 1.])
result = chigger.exodus.ExodusResult(reader, variable='diffused', camera=camera, cmap='viridis', filters=[contour])

window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('levels.png')
window.start()
