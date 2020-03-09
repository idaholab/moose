#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import vtk

line = vtk.vtkPlaneSource()
line.SetOrigin(1, 0, 1)
line.SetPoint1(2, 0, 1)
line.SetPoint2(1, 0, 2)

extrude = vtk.vtkRotationalExtrusionFilter()
extrude.SetResolution(20)
extrude.SetInputConnection(line.GetOutputPort())
extrude.SetAngle(90)

mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(extrude.GetOutputPort())

actor = vtk.vtkActor()
actor.SetMapper(mapper)

ren = vtk.vtkRenderer()
ren.AddActor(actor)
ren.SetBackground([0.5]*3)

win = vtk.vtkRenderWindow()
win.AddRenderer(ren)

iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(win)

iren.Initialize()
iren.Start()
