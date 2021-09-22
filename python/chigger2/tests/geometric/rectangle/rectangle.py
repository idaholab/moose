#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import chigger

window = chigger.Window(size=(300,300))
view = chigger.Viewport()
rect = chigger.geometric.Rectangle(xmin=0.25, xmax=0.75, ymin=0.25, ymax=0.75)
window.write(filename='rectangle.png')
window.start()

"""
import vtk
import numpy as np
n = 1000
m = 1

ydata = np.linspace(0, 1, n+1)
data = vtk.vtkFloatArray()
data.SetName('data')
data.SetNumberOfTuples((n+1)*(m+1))
idx = 0
for i in range(n+1):
    for j in range(m+1):
        data.SetValue(idx, 1-ydata[i])
        idx += 1

rect = chigger.geometric.Rectangle(cmap_name='viridis',
                                   bounds=(100, 200, 100, 200),
                                   #origin=(100, 100, 0),
                                   #point1=(150, 100, 0),
                                   #point2=(100, 200, 0),
                                   #origin=(0.25, 0.25, 0),
                                   #point1=(0.25, 0.75, 0),
                                   #point2=(0.3, 0.25, 0),
                                   resolution=(n,1),
                                   #color=(1,0,0),
                                   #linewidth=2,
                                   #cmap_reverse=True,
                                   rotate=45,
                                   #cmap_range=(0,0.5),
                                   #coordinate_system='viewport',
                                   point_data=data)
"""
