#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import os
import vtk
sys.path.append(os.path.abspath('../../python'))

import lsutils

camera = vtk.vtkCamera()
camera.SetViewUp(0.0000, 1.0000, 0.0000)
camera.SetPosition(0.5000, 0.5000, 1.8660)
camera.SetFocalPoint(0.5000, 0.5000, 0.0000)

lsutils.build_frames(camera=camera)
