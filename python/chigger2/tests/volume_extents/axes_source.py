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

extents = chigger.misc.VolumeAxesSource()
result = chigger.base.ChiggerResult(extents)
window = chigger.RenderWindow(result, size=(300,300), antialiasing=10)

window.write('axes_source.png')
window.start()
