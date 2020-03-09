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
#camera = vtk.vtkCamera()
#camera.SetViewUp(0.3405, 0.8395, -0.4234)
#camera.SetPosition(-1.7356, 2.8002, 3.3410)
#camera.SetFocalPoint(0.8096, 0.3052, 0.4410)

reader = chigger.exodus.MultiAppExodusReader('centroid_multiapp_out_sub*.e')
multiapp = chigger.exodus.ExodusResult(reader, variable='x', cmap='coolwarm', range=[0,1])

window = chigger.RenderWindow(multiapp, size=[300,300], test=False)
window.update()
window.resetCamera() # TODO: This is needed to re-center the object, not sure why
window.write('multiapp.png')
window.start()
