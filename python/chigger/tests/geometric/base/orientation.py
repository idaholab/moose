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
camera.SetViewUp(-0.1003, 0.9598, -0.2623)
camera.SetPosition(5.6199, 2.7342, 7.8553)
camera.SetFocalPoint(0.0000, 0.0000, 0.0000)

cyl0 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[0,0,0], color=[0.5,0,0], edges=False)
cyl1 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[90,0,0], color=[0,0.5,0], edges=False)
cyl2 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[0,0,90], color=[0,0,0.5], edges=False)

cyls = chigger.base.ChiggerResult(cyl0, cyl1, cyl2, camera=camera)

window = chigger.RenderWindow(cyls, size=[300,300], test=True)
window.write('orientation.png')
window.start()
