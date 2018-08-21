#!/usr/bin/env python2
import vtk
import random


class Move(object):
    def __init__(self, window):
        self.active = None
        self.window = window

    def onLeftButton(self, obj, event):

        if self.active is not None:
            print 'Remove'
            self.active = None

        else:
            print 'add'
            loc = obj.GetEventPosition()
            ren = obj.FindPokedRenderer(*loc)
            properties = ren.PickProp(*loc)
            if properties:
                self.active = properties.GetItemAsObject(0).GetViewProp()

    def onMouseMove(self, obj, event):
        loc = obj.GetEventPosition()

        if self.active is not None:
            #print loc
            self.active.SetPosition(*loc)
            self.window.Render()
            print self.active



        #print obj, event

# Create a rendering window and renderer.
ren = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.SetSize(1000, 1000)
renWin.AddRenderer(ren)

# Create a text actor.
txt = vtk.vtkTextActor()
txt.SetInput("Hello World!")
txt.SetDisplayPosition(20, 30)


print txt.GetProperty()

m = Move(renWin)

# Create a render window interactor.
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)
iren.AddObserver(vtk.vtkCommand.LeftButtonPressEvent, m.onLeftButton)
iren.AddObserver(vtk.vtkCommand.MouseMoveEvent, m.onMouseMove)

# Assign actor to the renderer.
ren.AddActor(txt)

# Enable user interface interactor.
iren.Initialize()
renWin.Render()
iren.Start()
