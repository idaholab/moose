#!/usr/bin/env python
import vtk

actor = vtk.vtkAxisActor2D()
actor.GetProperty().SetColor(0,0,0)
actor.SetPoint1(0.2, 0.2)
actor.SetPoint2(0.8, 0.8)
actor.GetLabelTextProperty().SetShadow(False)
actor.GetLabelTextProperty().SetColor(0,0,0)

actor.SetTitle("This is the title")
actor.GetTitleTextProperty().SetShadow(False)
actor.GetTitleTextProperty().SetColor(0,0,0)

# Why does setting orientation, shrink font size?
#actor.GetTitleTextProperty().SetOrientation(45)

renderer = vtk.vtkRenderer()
renderer.AddActor(actor)
renderer.SetBackground(1,1,1)

window = vtk.vtkRenderWindow()
window.AddRenderer(renderer)
window.SetSize(900, 900)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

window.Render()
interactor.Start()
