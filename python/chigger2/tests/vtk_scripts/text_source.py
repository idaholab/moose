#!/usr/bin/env python
import vtk

#source = vtk.vtkTextSource()
#source.SetText('Testing...')
#source.BackingOff()

#mapper = vtk.vtkPolyDataMapper2D()
#mapper.SetInputConnection(source.GetOutputPort())
#mapper.ScalarVisibilityOff()


actor = vtk.vtkTextActor()
actor.SetInput("$y = \sqrt{ax + b} + cx$")
actor.SetPosition(100, 100)
#actor.Update()

renderer = vtk.vtkRenderer()
renderer.AddActor(actor)

# Window and Interactor
window = vtk.vtkRenderWindow()
window.AddRenderer(renderer)
window.SetSize(400, 400)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

# Show the result
window.Render()

bbox = [0,0]
actor.GetSize(renderer, bbox)
print bbox
#print actor.GetMinimumSize(), actor.GetMaximumLineHeight()

interactor.Start()
