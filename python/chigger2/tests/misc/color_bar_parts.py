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
import numpy as np
import vtk
import chigger
from chigger import base, misc, geometric, annotations

n = 256
x = np.linspace(0, 1, n+1)

data = vtk.vtkFloatArray()
data.SetName('point_data')
data.SetNumberOfTuples((n+1)*2)
idx = 0
for i in range(n+1):
    for j in range(2):
        data.SetValue(idx, 1-x[i])
        idx += 1


# origin, width, length, angle

angle = 15
offset = 0.

box = geometric.Rectangle(origin=(0.3, 0.2, 0),
                          point1=(0.3, 0.25, 0),
                          point2=(0.8, 0.2, 0),
                          rotate=angle,
                          resolution=(1, 256),
                          point_data=data)
#box.setParams('transform', rotate=(90,0,0))


p0 = box.getParam('origin')
p1 = box._vtksource.GetPoint1()#getParam('point1')
p2 = box._vtksource.GetPoint2()#getParam('point2')

primary1 = (p1[0]-p0[0]+p2[0], p1[1]-p0[1]+p2[1])
primary2 = (p1[0], p1[1])

secondary1 = (p0[0], p0[1])
secondary2 = (p2[0], p2[1])


ax0 = misc.Axis2D(#title='Primary',
                  title_position=0.5,
                  title_orientation=45,
                  #point1=(0.899, 0.55),
                  #point2=(0.101, 0.55),
                  point1=primary1,
                  point2=primary2,
                  color=(0,0,0),
                  range=(2000,1998),
                  format='%.1f',
                  axis=True)

ax1 = misc.Axis2D(title='Secondary',
                  title_position=0.5,
                  point1=secondary1,
                  point2=secondary2,
                  color=(0,0,0),
                  range=(1980,1998),
                  format='%.1f',
                  axis=True)



p0 = box.getParam('origin')
p1 = box.getParam('point1')
p2 = box.getParam('point2')

voffset = 0.075
primary1 = (p1[0]-p0[0]+p2[0], p1[1]-p0[1]+p2[1])
primary2 = (p1[0], p1[1])

title0_pos = (primary2[0] + (primary1[0] - primary2[0])/2.,
              primary2[1] + (primary1[1] - primary2[1])/2. + voffset)

title0_pos =  geometric.Rectangle._rotatePoint(title0_pos, p0, angle)


title0 = annotations.Text('Primary', orientation=angle, halign='center', valign='bottom',
                          position=title0_pos)

comp = base.ChiggerCompositeSource(box, ax0, ax1, title0)


view = chigger.Viewport(comp)
window = chigger.Window(view, size=(600,600), background=(1,1,1))
window.write('color_bar_parts.png')
window.start()
