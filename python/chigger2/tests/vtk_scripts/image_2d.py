#!/usr/bin/env python
import vtk

reader = vtk.vtkPNGReader()
reader.SetFileName("../logos/moose.png")

#resize = vtk.vtkImageResize()
#resize.SetInputConnection(reader.GetOutputPort())
#resize.SetOutputDimensions(100, 100, 0)

mapper = vtk.vtkImageMapper()
mapper.SetInputConnection(reader.GetOutputPort())
#mapper.SetColorWindow(255); # width of the color range to map to
#mapper.SetColorLevel(127.5); # center of the color range to map to

# 2D Actor
actor = vtk.vtkActor2D()
actor.SetMapper(mapper)
#actor.SetPosition(0, 0.5)
#actor.SetPosition(0.75, 1)


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
