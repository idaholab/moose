#!/usr/bin/env python2
import vtk

# Text Mapper
mapper = vtk.vtkTextMapper()
mapper.SetInput("Testing...")

# 2D Actor
actor = vtk.vtkActor2D()
actor.SetMapper(mapper)

# Create a rendering window and renderer.
ren = vtk.vtkRenderer()
ren.AddActor(actor)

# Window
renWin = vtk.vtkRenderWindow()
renWin.SetSize(1000, 1000)
renWin.AddRenderer(ren)

# Create a render window interactor.
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# Enable user interface interactor.
iren.Initialize()
renWin.Render()
iren.Start()
