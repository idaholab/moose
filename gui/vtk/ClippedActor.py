from PeacockActor import PeacockActor

import vtk
from vtk.util.colors import peacock, tomato, red, white, black

class ClippedActor(PeacockActor):
  def __init__(self, original_actor, plane):
    PeacockActor.__init__(self, original_actor.renderer)
    self.original_actor = original_actor
    self.plane = plane

    self.clipper = vtk.vtkTableBasedClipDataSet()
    self.clipper.SetInput(self.original_actor.mesh)
    self.clipper.SetClipFunction(self.plane)
    self.clipper.Update()

    self.clip_mapper = vtk.vtkDataSetMapper()
    self.clip_mapper.SetInput(self.clipper.GetOutput())

    self.clip_actor = vtk.vtkActor()
    self.clip_actor.SetMapper(self.clip_mapper)

  def getBounds(self):
    return self.original_actor.getBounds()

  def movePlane(self):
    pass

  def _show(self):
    self.original_actor.renderer.AddActor(self.clip_actor)

  def _hide(self):
    self.original_actor.renderer.RemoveActor(self.clip_actor)

  def _showEdges(self):
    self.clip_actor.GetProperty().EdgeVisibilityOn()

  def _hideEdges(self):
    self.clip_actor.GetProperty().EdgeVisibilityOff()

  def _goSolid(self):
    self.clip_actor.GetProperty().SetRepresentationToSurface()

  def _goWireframe(self):
    self.clip_actor.GetProperty().SetRepresentationToWireframe()

  def _setColor(self, color):
    self.clip_actor.GetProperty().SetColor(color)
