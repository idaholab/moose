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

# Create camera
camera = vtk.vtkCamera()
camera.SetViewUp(-0.7, 0.5, 0.5)
camera.SetPosition(7, 0.2, 14)
camera.SetFocalPoint(0.0, 0.0, 0.125)

# Open the result
file_name = '../input/mug_blocks_out.e'
reader = chigger.exodus.ExodusReader(file_name, time=3)

mug = chigger.exodus.ExodusResult(reader, variable='convected', cmap='PiYG', camera=camera)
mug.setOptions('colorbar', visible=False)

# Create the window
window = chigger.RenderWindow(mug, size=[300,300], test=True)

# Render the results and write a file
window.write('simple_camera.png')
window.start()
