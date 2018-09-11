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

    return renderer

# Create some geometry for the left and right sides
left = create_renderer([0,0,0.5,1])
right = create_renderer([0.5,0,1,1])

# Window and Interactor
window = vtk.vtkRenderWindow()
window.SetSize(600, 600)
window.AddRenderer(left)
window.AddRenderer(right)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

# Disable interaction to begin, but create a interactor to be added while toggling results
interactor.SetInteractorStyle(None)
style = vtk.vtkInteractorStyleJoystickCamera()

def select(obj, event):
    """
    Function to select that toggles through the available renderers
    """
    key = obj.GetKeySym()
    if key == 't':
        current = style.GetCurrentRenderer()
        if current is None:
            print 'Left Side Active'
            style.SetCurrentRenderer(left)
            style.HighlightProp3D(left.GetActors().GetLastActor())
            interactor.SetInteractorStyle(style)
            # HELP: Disable right-side renderer from being manipulated...nothing below works
            right.GetActors().GetLastActor().SetPickable(False)
            right.GetActors().GetLastActor().SetDragable(False)
            right.SetInteractive(False)

        elif current == left:
            print 'Right Side Active'
            style.SetCurrentRenderer(right)
            style.HighlightProp3D(right.GetActors().GetLastActor())
            interactor.SetInteractorStyle(style)
            # HELP: Disable left-side renderer from being manipulated...

        else:
            print 'Nothing Active'
            style.HighlightProp3D(None)
            style.SetCurrentRenderer(None)
            interactor.SetInteractorStyle(None)
    window.Render()

interactor.AddObserver(vtk.vtkCommand.KeyPressEvent, select)

# Show the result
window.Render()
interactor.Start()
