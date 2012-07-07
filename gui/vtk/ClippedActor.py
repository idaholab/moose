from PeacockActor import PeacockActor

import vtk
from vtk.util.colors import peacock, tomato, red, white, black

class ClippedActor(PeacockActor):
  def __init__(self, original_actor, plane):
    PeacockActor.__init__(self, original_actor.renderer)
    self.original_actor = original_actor
    self.plane = plane

    self.clipper = vtk.vtkClipPolyData()
    self.clipper.SetInput(self.original_actor.geom.GetOutput())
    self.clipper.SetClipFunction(self.plane)
    self.clipper.GenerateClipScalarsOn()

    self.clip_mapper = vtk.vtkPolyDataMapper()
    self.clip_mapper.SetInputConnection(self.clipper.GetOutputPort())
    self.clip_mapper.ScalarVisibilityOff()

    self.clip_actor = vtk.vtkActor()
    self.clip_actor.SetMapper(self.clip_mapper)

#     self.cutter = vtk.vtkCutter()
#     self.cutter.SetInput(self.original_actor.geom.GetOutput())
#     self.cutter.SetCutFunction(self.plane)
#     self.cutter.GenerateCutScalarsOn()
    
#     self.stripper = vtk.vtkStripper()
#     self.stripper.SetInputConnection(self.cutter.GetOutputPort())
#     self.stripper.Update()
    
#     self.cut_poly = vtk.vtkPolyData()
#     self.cut_poly.SetPoints(self.stripper.GetOutput().GetPoints())
#     self.cut_poly.SetPolys(self.stripper.GetOutput().GetLines())

#     self.cut_triangles = vtk.vtkTriangleFilter()
#     self.cut_triangles.SetInput(self.cut_poly)
#     self.cut_mapper = vtk.vtkPolyDataMapper()
#     self.cut_mapper.SetInput(self.cut_poly)
#     self.cut_mapper.SetInputConnection(self.cut_triangles.GetOutputPort())
#     self.cut_actor = vtk.vtkActor()
#     self.cut_actor.SetMapper(self.cut_mapper)

  def getBounds(self):
    return self.original_actor.getBounds()

  def movePlane(self):
    pass
#    self.stripper.Update()
#    self.cut_poly.SetPoints(self.stripper.GetOutput().GetPoints())
#    self.cut_poly.SetPolys(self.stripper.GetOutput().GetLines())
#    self.cut_mapper.Update()
  
  def _show(self):
    self.original_actor.renderer.AddActor(self.clip_actor)
#    self.original_actor.renderer.AddActor(self.cut_actor)

  def _hide(self):
    self.original_actor.renderer.RemoveActor(self.clip_actor)
#    self.original_actor.renderer.RemoveActor(self.cut_actor)

  def _showEdges(self):
    self.clip_actor.GetProperty().EdgeVisibilityOn()
#    self.cut_actor.GetProperty().EdgeVisibilityOn()
    
  def _hideEdges(self):
    self.clip_actor.GetProperty().EdgeVisibilityOff()
#    self.cut_actor.GetProperty().EdgeVisibilityOff()

  def _goSolid(self):
    self.clip_actor.GetProperty().SetRepresentationToSurface()
#    self.original_actor.renderer.AddActor(self.cut_actor)
    
#    self.cut_actor.GetProperty().SetRepresentationToSurface()
    
  def _goWireframe(self):
    self.clip_actor.GetProperty().SetRepresentationToWireframe()
#    self.original_actor.renderer.RemoveActor(self.cut_actor)
#    self.cut_actor.GetProperty().SetRepresentationToWireframe()

  def _setColor(self, color):
    self.clip_actor.GetProperty().SetColor(color)
#    self.cut_actor.GetProperty().SetColor(color)
