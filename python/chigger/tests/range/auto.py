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
camera.SetViewUp(0.0000, 1.0000, 0.0000)
camera.SetPosition(0.6952, 0.5049, 2.7321)
camera.SetFocalPoint(0.6952, 0.5049, 0.0000)

reader = chigger.exodus.ExodusReader('../input/variable_range.e', timestep=0)
reader.update()
mug = chigger.exodus.ExodusResult(reader, variable='u', camera=camera)
cbar = chigger.exodus.ExodusColorBar(mug, colorbar_origin=[0.7,0.25])
window = chigger.RenderWindow(mug, cbar, size=[300,300], test=True)

# Render the results and write a file
t = reader.getTimes()
for i in range(len(t)):
    reader.setOptions(timestep=i)
    window.write('auto_' + str(i) + '.png')
window.start()
