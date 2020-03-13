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
camera.SetViewUp(0.1865, 0.6455, 0.7407)
camera.SetPosition(3.7586, -11.8847, 9.5357)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
exodus0 = chigger.exodus.ExodusSource(reader, block=['1'])
exodus0.update()
exodus1 = chigger.exodus.ExodusSource(reader, block=['76'], edges=True, edge_color=[1,0,0], edge_width=1)
exodus1.update()

result = chigger.base.ChiggerResult(exodus0, exodus1, variable='diffused', camera=camera)
window = chigger.RenderWindow(result, size=[300, 300], test=True)
window.update(); window.resetCamera()
window.write('edge.png')
window.start()
