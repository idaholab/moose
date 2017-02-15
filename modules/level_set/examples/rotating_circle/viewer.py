#!/usr/bin/env python
import sys
import os
import vtk
sys.path.append(os.path.abspath('../../python'))

import lsutils

camera = vtk.vtkCamera()
camera.SetViewUp(0.0000, 1.0000, 0.0000)
camera.SetPosition(0.0000, 0.0000, 3.5431)
camera.SetFocalPoint(0.0000, 0.0000, 0.0000)

lsutils.build_frames(camera=camera)
