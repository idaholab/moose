#!/usr/bin/env python
import vtk

source = vtk.vtkPlaneSource()
source.SetOrigin(200, 100, 0)
source.SetPoint1(300, 200, 0)
source.SetPoint2(100, 200, 0)
source.Update()

mapper = vtk.vtkPolyDataMapper2D()
mapper.SetInputConnection(source.GetOutputPort())

actor = vtk.vtkActor2D()
actor.SetMapper(mapper)
actor.GetProperty().SetColor(0.16,0.2,0.1)

outline = vtk.vtkOutlineSource()
outline.SetBounds(source.GetOutput().GetBounds())
#outline.SetInputConnection(source.GetOutputPort())
#outline.GetProperty().SetColor(1,0,0)

mapper2 = vtk.vtkPolyDataMapper2D()
mapper2.SetInputConnection(outline.GetOutputPort())

actor2 = vtk.vtkActor2D()
actor2.SetMapper(mapper2)
actor2.GetProperty().SetColor(1,0.2,0.1)

renderer = vtk.vtkRenderer()
renderer.AddActor(actor)
renderer.AddActor(actor2)

# Window and Interactor
window = vtk.vtkRenderWindow()
window.AddRenderer(renderer)
window.SetSize(400, 400)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
#interactor.GetInteractorStyle().HighlightProp(actor)
interactor.Initialize()

# Show the result
window.Render()
interactor.Start()
