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
camera.SetViewUp(0.0105, 0.1507, 0.9885)
camera.SetPosition(15.6131, -0.3930, 0.0186)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, block=[76], representation='points', camera=camera, color=[0,1,0])
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.update();window.resetCamera() #TODO: This is needed to make results render correctly, not sure why
window.write('points.png')
window.start()
