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
camera.SetViewUp(-0.0673, 0.8897, 0.4516)
camera.SetPosition(-5.3701, -6.9590, 13.0350)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', camera=camera, cmap='viridis')

extents = chigger.misc.VolumeAxes(mug)
extents.setOptions('xaxis', color=[1,0,0], minor_ticks=True)
window = chigger.RenderWindow(mug, extents, size=[300,300], antialiasing=10, test=True)

window.write('extents.png')
window.start()
