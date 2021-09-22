#!/usr/bin/env python
"""
Script to demonstrate toggling interaction between two actors on one renderer.
"""
import vtk

class Chigger3DInteractorStyle(vtk.vtkInteractorStyleMultiTouchCamera):
    pass

class Chigger2DInteractorStyle(vtk.vtkInteractorStyleUser):
    ZOOM_FACTOR = 2

    def __init__(self, source):
        self.AddObserver(vtk.vtkCommand.MouseWheelForwardEvent, self.onMouseWheelForward)
        self.AddObserver(vtk.vtkCommand.MouseWheelBackwardEvent, self.onMouseWheelBackward)
        self.AddObserver(vtk.vtkCommand.KeyPressEvent, self.onKeyPressEvent)
        self.AddObserver(vtk.vtkCommand.MouseMoveEvent, self.onMouseMoveEvent)

        super(Chigger2DInteractorStyle, self).__init__()

        self._source = source
        self._move_origin = None

    def onMouseWheelForward(self, obj, event):
        self.zoom(self.ZOOM_FACTOR)
        obj.GetInteractor().GetRenderWindow().Render()

    def onMouseWheelBackward(self, obj, event):
        self.zoom(-self.ZOOM_FACTOR)
        obj.GetInteractor().GetRenderWindow().Render()

    def onKeyPressEvent(self, obj, event):
        key = obj.GetKeySym().lower()
        if key == 'shift_l':
            self._shift_origin = obj.GetInteractor().GetEventPosition()

    def onMouseMoveEvent(self, obj, event):
        if obj.GetShiftKey():
            if self._move_origin == None:
                self._move_origin = obj.GetInteractor().GetEventPosition()
            else:
                pos = obj.GetInteractor().GetEventPosition()
                self.move(pos[0] - self._move_origin[0], pos[1] - self._move_origin[1])
                obj.GetInteractor().GetRenderWindow().Render()
                self._move_origin = pos

    def zoom(self, factor):

        origin = self._source.GetOrigin()
        self._source.SetOrigin([origin[0] + factor, origin[1] + factor, 0])

        p = self._source.GetPoint1()
        self._source.SetPoint1([p[0] + factor, p[1] - factor, 0])

        p = self._source.GetPoint2()
        self._source.SetPoint2([p[0] - factor, p[1] + factor, 0])

    def move(self, dx, dy):

        origin = self._source.GetOrigin()
        self._source.SetOrigin([origin[0] + dx, origin[1] + dy, 0])

        p = self._source.GetPoint1()
        self._source.SetPoint1([p[0] + dx, p[1] + dy, 0])

        p = self._source.GetPoint2()
        self._source.SetPoint2([p[0] + dx, p[1] + dy, 0])


source1 = vtk.vtkPlaneSource()
source1.SetOrigin([100, 100, 0])
source1.SetPoint1([100, 300, 0])
source1.SetPoint2([300, 100, 0])

mapper1 = vtk.vtkPolyDataMapper2D()
mapper1.SetInputConnection(source1.GetOutputPort())

actor1 = vtk.vtkActor2D()
actor1.SetMapper(mapper1)

source2 = vtk.vtkCubeSource()
source2.SetBounds(0,1,0,1,0,1)

mapper2 = vtk.vtkPolyDataMapper()
mapper2.SetInputConnection(source2.GetOutputPort())

actor2 = vtk.vtkActor()
actor2.SetMapper(mapper2)

renderer = vtk.vtkRenderer()
renderer.AddActor(actor1)
renderer.AddActor(actor2)

window = vtk.vtkRenderWindow()
window.SetSize(600, 600)
window.AddRenderer(renderer)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(window)

style1 = Chigger2DInteractorStyle(source1)
style2 = Chigger3DInteractorStyle()
interactor.SetInteractorStyle(style1)
interactor.Initialize()





window.Render()
interactor.Start()
