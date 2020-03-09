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

n = 10        # Number of data points to create
name = "data" # Name of data array generated

# A Line
source = vtk.vtkLineSource()
source.SetResolution(n)
source.SetPoint1(0, 0, 0)
source.SetPoint2(0, 1, 0)
source.SetResolution(n)

# Create and apply nonlinear data along the line
data = vtk.vtkFloatArray()
data.SetName(name)
data.SetNumberOfTuples(n+1)
for i in range(n+1):
    data.SetValue(i, i*i)

source.Update()
source.GetOutput().GetPointData().AddArray(data)

# Build tube
tube = vtk.vtkTubeFilter()
tube.SetInputConnection(source.GetOutputPort())
tube.SetRadius(0.2)
tube.SetNumberOfSides(100)
tube.SetCapping(True)

mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(tube.GetOutputPort())
mapper.SetScalarModeToUsePointFieldData()
mapper.SelectColorArray(name)
mapper.SetScalarRange(data.GetRange(0))

actor = vtk.vtkActor()
actor.SetMapper(mapper)

renderer = vtk.vtkRenderer()
renderer.AddActor(actor)

window = vtk.vtkRenderWindow()
window.AddRenderer(renderer)
window.SetSize(500, 500)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

window.Render()
interactor.Start()
