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
camera.SetViewUp(-0.2551037, 0.6735653, 0.6937088)
camera.SetPosition(11.4380361, -5.5098613, 9.6046531)
camera.SetFocalPoint(0.6296384, -0.5413410, 0.8057375)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')

mug = chigger.exodus.ExodusResult(reader, camera=camera, block=['1'], cmap='shock', variable='diffused')
mug.setOptions('colorbar', colorbar_origin=[0.05,0.05], length=0.3, location='top')

contour = chigger.filters.ContourFilter()
contour = chigger.exodus.ExodusResult(reader, camera=camera, block=['76'], variable='convected', cmap='viridis', filters=[contour])

window = chigger.RenderWindow(mug, contour, size=[600,400], test=True)

window.write('combo.png')
window.start()
