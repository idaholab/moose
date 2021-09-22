#!/usr/bin/env python
import vtk

# Create a Cube
cube = vtk.vtkCubeSource()
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(cube.GetOutputPort())
actor = vtk.vtkActor()
actor.SetMapper(mapper)

# Renderer
renderer = vtk.vtkRenderer()
renderer.AddViewProp(actor)

# Window and Interactor
window = vtk.vtkRenderWindow()
window.AddRenderer(renderer)
window.SetSize(600, 600)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

# Create a timer
def manipulate(*args):
    print("manipulate")
    writer = vtk.vtkPNGWriter()
    window_to_image = vtk.vtkWindowToImageFilter()
    window_to_image.SetInput(window)
    writer.SetInputData(window_to_image.GetOutput())

    # Save the un-changed rendering
    window.Render()
    window_to_image.Update()
    writer.SetFileName("before.png")
    writer.Write()

    # How to manipulate the scene pragmatically? For example click such that the cube rotates;
    # on MacOS just clicking down on the cube causes it to rotate, how can that be mimiced here?
    interactor.SetEventPosition(50, 50)
    interactor.LongTapEvent()

    # Save the the manipulated rendering
    window.Render()
    window_to_image.Update()
    writer.SetFileName("after.png")
    writer.Write()

# Call the above function after the window shows up
interactor.CreateOneShotTimer(1000)
interactor.AddObserver(vtk.vtkCommand.TimerEvent, manipulate)

# Show the result
window.Render()
interactor.Start()
