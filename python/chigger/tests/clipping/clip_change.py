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
camera.SetViewUp(0.2603, 0.5500, 0.7936)
camera.SetPosition(-12.3985, 5.2867, 0.8286)
camera.SetFocalPoint(0.2326, 0.0324, 0.3278)

clip = chigger.filters.PlaneClipper(normal=[1,1,1], normalized=False)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, cmap='viridis', variable='diffused', camera=camera, range=[0,2], filters=[clip])

# Create the window
window = chigger.RenderWindow(mug, size=[300,300], test=True)

# Render the results and write a file
steps = [-1, 0, 1]
for i in range(len(steps)):
    clip.setOptions(origin=[steps[i]]*3)
    window.write('clip_change' + str(i) + '.png')
window.start()
