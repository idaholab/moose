#!/usr/bin/env python
import vtk

# Object
source = vtk.vtkCylinderSource()
source.SetResolution(3)
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(source.GetOutputPort())
actor = vtk.vtkActor()
actor.SetMapper(mapper)


# Outline
outline_source = vtk.vtkOutlineSource()
outline_source.SetBounds(actor.GetBounds())
outline_mapper = vtk.vtkPolyDataMapper()
outline_mapper.SetInputConnection(outline_source.GetOutputPort())
outline_actor = vtk.vtkActor()
outline_actor.SetMapper(outline_mapper)
outline_actor.GetProperty().SetColor(1,0,0)
outline_actor.SetVisibility(False)

# Renderer
renderer = vtk.vtkRenderer()
renderer.AddActor(actor)
renderer.AddActor(outline_actor)

# Window/Interactor
window = vtk.vtkRenderWindow()
window.SetSize(600, 600)
window.AddRenderer(renderer)
interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)
interactor.Initialize()

# HELP: I want to disable the ability to reposition the object, but want keybindings to operate.
#actor.SetDragable(False)
#actor.SetPickable(False)
#renderer.SetInteractive(False)
#interactor.GetInteractorStyle().SetDefaultRenderer(None)
#interactor.GetInteractorStyle().SetCurrentRenderer(None)
style = vtk.vtkInteractorStyleJoystickCamera()
interactor.SetInteractorStyle(None)


# Interaction toggle
def select(obj, event):
    """Ensble/Disable the interaction with the object."""
    key = obj.GetKeySym()
    if key == 't':
        value = not outline_actor.GetVisibility()
        outline_actor.SetVisibility(value)
        if value:
            interactor.SetInteractorStyle(style)
        else:
            interactor.SetInteractorStyle(None)

            # Enable the mouse manipulation
    window.Render()
interactor.AddObserver(vtk.vtkCommand.KeyPressEvent, select)

window.Render()
interactor.Start()
