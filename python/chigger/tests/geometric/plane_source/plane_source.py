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
#from mooseutils import message
#message.MOOSE_DEBUG_MODE = True

data = vtk.vtkFloatArray()
n = 100
m = 100
data.SetNumberOfTuples(n*m)
idx = 0
for i in range(n):
    for j in range(m):
        data.SetValue(idx, i+j)
        idx += 1

plane0 = chigger.geometric.PlaneSource2D(origin=[100,100,0], point1=[100,200,0], point2=[200,100,0], resolution=[n,m], cmap='viridis', data=data)
result = chigger.base.ChiggerResult(plane0)
window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('plane_source.png')
window.start()
