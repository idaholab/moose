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

# Create a common camera
camera = vtk.vtkCamera()
camera.SetViewUp(0.1889, 0.9412, -0.2800)
camera.SetPosition(3.4055, -0.8236, -1.9897)
camera.SetFocalPoint(0.5000, 0.5000, 0.5000)

reader = chigger.exodus.NemesisReader('../input/nemesis.e.24.*')
result = chigger.exodus.ExodusResult(reader, variable='u', explode=0.3, camera=camera)

window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('explode.png')
window.start()
