#!/usr/bin/env python
"""
Script to demonstrate toggling interaction between two renderers.
"""
import vtk

def create_renderer(view):
    """
    Helper to adding a source to the window.
    """
    source = vtk.vtkCylinderSource()
    source.SetResolution(3)

    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())

    actor = vtk.vtkActor()
    actor.SetMapper(mapper)

    renderer = vtk.vtkRenderer()
    renderer.AddActor(actor)
    renderer.SetViewport(view)
    renderer.SetLayer(1)
    renderer.SetInteractive(False)

    return renderer

# Create some geometry for the left and right sides
left = create_renderer([0,0,0.5,1])
right = left#create_renderer([0.5,0,1,1])

background = vtk.vtkRenderer()
background.SetLayer(0)
background.SetInteractive(True)

# Window and Interactor
window = vtk.vtkRenderWindow()
window.SetSize(600, 600)
window.AddRenderer(background) # used vtkRenderWindowInteractor::FindPokedRenderer as last resort
window.AddRenderer(left)
window.AddRenderer(right)
window.SetNumberOfLayers(2)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

# Disable interaction to begin, but create a interactor to be added while toggling results
style = vtk.vtkInteractorStyleJoystickCamera()
interactor.SetInteractorStyle(style)
style.SetInteractor(interactor)
style.SetCurrentRenderer(background)
style.SetEnabled(False)

def select(obj, event):
    """
    Function to select that toggles through the available renderers
    """
    key = obj.GetKeySym()
    if key == 't':
        if left.GetInteractive():
            style.SetCurrentRenderer(right)
            style.SetEnabled(True)
            left.SetInteractive(False)
            right.SetInteractive(True)
            style.HighlightProp3D(right.GetActors().GetLastActor())

        elif right.GetInteractive():
            style.SetCurrentRenderer(background)
            style.SetEnabled(False)
            left.SetInteractive(False)
            right.SetInteractive(False)
            style.HighlightProp3D(None)

        else:
            style.SetCurrentRenderer(left)
            style.SetEnabled(True)
            left.SetInteractive(True)
            right.SetInteractive(False)
            style.HighlightProp3D(left.GetActors().GetLastActor())

    window.Render()

interactor.AddObserver(vtk.vtkCommand.KeyPressEvent, select)

# Show the result
window.Render()
interactor.Start()
