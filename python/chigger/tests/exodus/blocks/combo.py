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
camera.SetViewUp(-0.1625, 0.8440, 0.5111)
camera.SetPosition(-6.3934, -8.1737, 11.5260)
camera.SetFocalPoint(0.1695, 0.0712, -0.0016)

reader = chigger.exodus.ExodusReader('../../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, block=['76'], boundary=['top'], nodeset=['1'], variable='convected', cmap='viridis', camera=camera)
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('combo.png')
window.start()
