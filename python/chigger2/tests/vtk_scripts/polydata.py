#!/usr/bin/env python
import vtk
import numpy as np

name = 'data'
n = 256
point1 = [0.25, 0.25]
point2 = [0.5, 0.75]

ydata = np.linspace(point1[1], point2[1], n+1)

# Define points
points = vtk.vtkPoints()
points.SetNumberOfPoints(len(ydata)*2)

data = vtk.vtkFloatArray()
data.SetName(name)
data.SetNumberOfTuples(len(ydata)*2)

cells = vtk.vtkCellArray()
#cells.SetNumberOfCells(n)

idx = 0
for i, y in enumerate(ydata):
    points.SetPoint(idx, point1[0], y, 0)
    points.SetPoint(idx+1, point2[0], y, 0)

    value = float(i)/(n)
    data.SetValue(idx, value)
    data.SetValue(idx+1, value)

    if idx > 0:
        quad = vtk.vtkQuad()
        quad.GetPointIds().SetId(0, idx-2)
        quad.GetPointIds().SetId(1, idx-1)
        quad.GetPointIds().SetId(2, idx+1)
        quad.GetPointIds().SetId(3, idx)
        cells.InsertNextCell(quad)
        #cells.InsertNextCell(vtk.VTK_QUAD, [idx-2, idx-1, idx, idx+1])
    idx += 2

poly = vtk.vtkPolyData()
poly.SetPoints(points)
poly.SetPolys(cells)
poly.GetPointData().AddArray(data)

coordinate = vtk.vtkCoordinate()
coordinate.SetCoordinateSystemToNormalizedDisplay()

mapper = vtk.vtkPolyDataMapper2D()
mapper.SetInputData(poly)
mapper.SetScalarModeToUsePointFieldData()
#mapper.SetScalarRange(0, 1)
mapper.SetTransformCoordinate(coordinate)
mapper.ColorByArrayComponent(name, 0)
actor = vtk.vtkActor2D()
actor.SetMapper(mapper)

renderer = vtk.vtkRenderer()
renderer.AddActor2D(actor)

window = vtk.vtkRenderWindow()
window.AddRenderer(renderer)
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(window)
iren.Start()
