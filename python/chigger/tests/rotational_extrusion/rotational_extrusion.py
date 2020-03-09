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
camera.SetViewUp(-0.0124, 0.7882, 0.6153)
camera.SetPosition(0.0783, -0.1549, 0.3130)
camera.SetFocalPoint(0.0500, 0.0500, 0.0500)

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e')

rotate = chigger.filters.TransformFilter(rotate=[90,0,0]) # move to x-z plane
extrude = chigger.filters.RotationalExtrusionFilter(angle=90) # rotate along z

result = chigger.exodus.ExodusResult(reader, filters=[rotate, extrude], camera=camera)
window = chigger.RenderWindow(result, size=[300,300], antialiasing=10, test=True)
window.write('rotational_extrusion.png')
window.start()
