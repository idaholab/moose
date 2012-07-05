import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
from vtk.util.colors import peacock, tomato, red, white

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class ExodusMap:
  # These are the blocks from the multiblockdataset that correspond to each item
  element_vtk_block = 0
  sideset_vtk_block = 4
  nodeset_vtk_block = 7

class ExodusActor:
  def __init__(self, renderer, data, type, index):
    self.renderer = renderer
    self.data = data
    self.type = type
    self.index = index

    self.solid_visible = False
    self.edges_visible = False
    
    self.mesh = data.GetBlock(type).GetBlock(index)
    
    self.geom = vtk.vtkGeometryFilter()
    self.geom.SetInput(self.mesh)
    self.geom.Update()
    
    self.mapper = vtk.vtkDataSetMapper()
    self.mapper.SetInput(self.mesh)
    
    self.actor = vtk.vtkActor()
    self.actor.SetMapper(self.mapper)

    self.edges = vtk.vtkExtractEdges()
    self.edges.SetInput(self.mesh)
    self.edge_mapper = vtk.vtkPolyDataMapper()
    self.edge_mapper.SetInput(self.edges.GetOutput())

    self.edge_actor = vtk.vtkActor()
    self.edge_actor.SetMapper(self.edge_mapper)
    self.edge_actor.GetProperty().SetColor(0,0,0)

  def showSolid(self):
    self.solid_visible = True
    self.renderer.AddActor(self.actor)

  def hideSolid(self):
    self.solid_visible = False
    self.renderer.RemoveActor(self.actor)

  def showEdges(self):
    self.edges_visible = True
    self.renderer.AddActor(self.edge_actor)
    
  def hideEdges(self):
    self.edges_visible = False
    self.renderer.RemoveActor(self.edge_actor)

  def setSolidColor(self, color):
    self.actor.GetProperty().SetColor(color)

class ClippedActor:
  def __init__(self, original_actor, plane):
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

    self.cutter = vtk.vtkCutter()
    self.cutter.SetInput(self.original_actor.geom.GetOutput())
    self.cutter.SetCutFunction(self.plane)
    self.cutter.GenerateCutScalarsOn()
    
    self.stripper = vtk.vtkStripper()
    self.stripper.SetInputConnection(self.cutter.GetOutputPort())
    self.stripper.Update()
    
    self.cut_poly = vtk.vtkPolyData()
    self.cut_poly.SetPoints(self.stripper.GetOutput().GetPoints())
    self.cut_poly.SetPolys(self.stripper.GetOutput().GetLines())

    self.cut_triangles = vtk.vtkTriangleFilter()
    self.cut_triangles.SetInput(self.cut_poly)
    self.cut_mapper = vtk.vtkPolyDataMapper()
    self.cut_mapper.SetInput(self.cut_poly)
    self.cut_mapper.SetInputConnection(self.cut_triangles.GetOutputPort())
    self.cut_actor = vtk.vtkActor()
    self.cut_actor.SetMapper(self.cut_mapper)

    self.edge_clipper = vtk.vtkClipPolyData()
    self.edge_clipper.SetInput(self.original_actor.edges.GetOutput())
    self.edge_clipper.SetClipFunction(self.plane)
    self.edge_clipper.GenerateClipScalarsOn()

    self.edge_clip_mapper = vtk.vtkPolyDataMapper()
    self.edge_clip_mapper.SetInputConnection(self.edge_clipper.GetOutputPort())
    self.edge_clip_mapper.ScalarVisibilityOff()

    self.edge_clip_actor = vtk.vtkActor()
    self.edge_clip_actor.SetMapper(self.edge_clip_mapper)

    self.edge_cutter = vtk.vtkCutter()
    self.edge_cutter.SetInput(self.original_actor.edges.GetOutput())
    self.edge_cutter.SetCutFunction(self.plane)
    self.edge_cutter.GenerateCutScalarsOn()
    
    self.edge_stripper = vtk.vtkStripper()
    self.edge_stripper.SetInputConnection(self.edge_cutter.GetOutputPort())
    self.edge_stripper.Update()
    
    self.edge_cut_poly = vtk.vtkPolyData()
    self.edge_cut_poly.SetPoints(self.edge_stripper.GetOutput().GetPoints())
    self.edge_cut_poly.SetPolys(self.edge_stripper.GetOutput().GetLines())

    self.edge_cut_triangles = vtk.vtkTriangleFilter()
    self.edge_cut_triangles.SetInput(self.edge_cut_poly)
    self.edge_cut_mapper = vtk.vtkPolyDataMapper()
    self.edge_cut_mapper.SetInput(self.edge_cut_poly)
    self.edge_cut_mapper.SetInputConnection(self.edge_cut_triangles.GetOutputPort())
    self.edge_cut_actor = vtk.vtkActor()
    self.edge_cut_actor.SetMapper(self.edge_cut_mapper)

  def movePlane(self, distance):
    self.clipper.SetValue(distance)
    self.cutter.SetValue(0, distance)
    self.stripper.Update()
    self.cut_poly.SetPoints(self.stripper.GetOutput().GetPoints())
    self.cut_poly.SetPolys(self.stripper.GetOutput().GetLines())
    self.cut_mapper.Update()
  
  def showSolid(self):
    self.original_actor.renderer.AddActor(self.clip_actor)
    self.original_actor.renderer.AddActor(self.cut_actor)

  def hideSolid(self):
    self.original_actor.renderer.RemoveActor(self.clip_actor)
    self.original_actor.renderer.RemoveActor(self.cut_actor)

  def showEdges(self):
    self.original_actor.renderer.AddActor(self.edge_clip_actor)
    self.original_actor.renderer.AddActor(self.edge_cut_actor)

  def hideEdges(self):
    self.original_actor.renderer.RemoveActor(self.edge_clip_actor)
    self.original_actor.renderer.RemoveActor(self.edge_cut_actor)

class ExodusRenderWidget(QtGui.QWidget):
  def __init__(self):
    QtGui.QWidget.__init__(self)
    self.this_layout = QtGui.QVBoxLayout()
    self.setMinimumWidth(700)
    self.setLayout(self.this_layout)

    self.vtkwidget = vtk.QVTKWidget2()

    self.renderer = vtk.vtkRenderer()
    self.renderer.SetBackground(0,0,0)
    self.renderer.SetBackground2(1,1,1)
    self.renderer.SetGradientBackground(1)
    self.renderer.ResetCamera()
    
    self.this_layout.addWidget(self.vtkwidget)
    self.vtkwidget.show()    

    self.current_actors = []

    self.vtkwidget.GetRenderWindow().AddRenderer(self.renderer)
    

  def setFileName(self, file_name):
    for actor in self.current_actors:
      self.renderer.RemoveActor(actor)

    del self.current_actors[:]
      
    self.currently_has_actor = True
    
    self.file_name = file_name
    reader = vtk.vtkExodusIIReader()
    reader.SetFileName(self.file_name)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.EDGE_SET, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.SIDE_SET, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODE_SET, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL_TEMPORAL, 1)
    reader.UpdateInformation()
    reader.SetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 0, 1)

    num_sidesets = reader.GetNumberOfSideSetArrays()
    num_nodesets = reader.GetNumberOfNodeSetArrays()
    num_blocks = reader.GetNumberOfElementBlockArrays()

    self.sidesets = []
    self.sideset_id_to_exodus_block = {}
    for i in xrange(num_sidesets):
      sideset_id = reader.GetObjectId(vtk.vtkExodusIIReader.SIDE_SET,i)
      self.sidesets.append(sideset_id)
      self.sideset_id_to_exodus_block[sideset_id] = i
      reader.SetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, i, 1)

    self.nodesets = []
    self.nodeset_id_to_exodus_block = {}
    for i in xrange(num_nodesets):
      nodeset_id = reader.GetObjectId(vtk.vtkExodusIIReader.SIDE_SET,i)
      self.nodesets.append(nodeset_id)
      self.nodeset_id_to_exodus_block[nodeset_id] = i
      reader.SetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, i, 1)

    self.blocks = []
    self.block_id_to_exodus_block = {}
    for i in xrange(num_blocks):
      block_id = reader.GetObjectId(vtk.vtkExodusIIReader.ELEM_BLOCK,i)
      self.blocks.append(block_id)
      self.block_id_to_exodus_block[block_id] = i
    
    reader.SetTimeStep(1)
    reader.Update()

    self.data = reader.GetOutput()

    self.sideset_actors = {}
    for i in xrange(num_sidesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.sideset_vtk_block, i)
      actor.setSolidColor(red)
      self.sideset_actors[str(self.sidesets[i])] = actor

    self.nodeset_actors = {}
    for i in xrange(num_nodesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.nodeset_vtk_block, i)
      actor.setSolidColor(red)
      self.nodeset_actors[str(self.nodesets[i])] = actor

    self.block_actors = {}
    for i in xrange(num_blocks):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.element_vtk_block, i)
      self.block_actors[str(self.blocks[i])] = actor
      actor.showSolid()
      actor.showEdges()

    plane = vtk.vtkPlane()
    plane.SetOrigin(0, 0, 0)
    plane.SetNormal(1, 0, 0)
    
#    self.ca = ClippedActor(self.block_actors[2], plane)
#    self.ca.showEdges()
#    self.sideset_actors[1].showSolid()

    # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()

  def highlightBoundary(self, boundary):
    # Turn off all sidesets
    for actor_name, actor in self.sideset_actors.items():
      actor.hideSolid()
      actor.hideEdges()

    # Turn off all nodesets
    for actor_name, actor in self.nodeset_actors.items():
      actor.hideSolid()
      actor.hideEdges()

    # Turn solids to only edges... but only if they are visible
    for actor_name, actor in self.block_actors.items():
      if actor.solid_visible:
        actor.hideSolid()
        actor.showEdges()

    boundaries = boundary.strip("'").split(' ')
    for the_boundary in boundaries:
      if the_boundary in self.sideset_actors:
        self.sideset_actors[the_boundary].showSolid()
    
    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()

  def highlightBlock(self, block):
    # Turn off all sidesets
    for actor_name, actor in self.sideset_actors.items():
      actor.hideSolid()
      actor.hideEdges()

    # Turn off all nodesets
    for actor_name, actor in self.nodeset_actors.items():
      actor.hideSolid()
      actor.hideEdges()

    # Turn solids to only edges... but only if they are visible
    for actor_name, actor in self.block_actors.items():
      if actor.solid_visible:
        actor.hideSolid()
        actor.showEdges()

    blocks = block.strip("'").split(' ')
    for the_block in blocks:
      if the_block in self.block_actors:
        self.block_actors[the_block].setSolidColor(red)
        self.block_actors[the_block].showSolid()
    
    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()

  def clearHighlight(self):
    # Turn off all sidesets
    for actor_name, actor in self.sideset_actors.items():
      actor.hideSolid()
      actor.hideEdges()

    # Turn off all nodesets
    for actor_name, actor in self.nodeset_actors.items():
      actor.hideSolid()
      actor.hideEdges()

    # Show solids and edges - but only if something is visible
    for actor_name, actor in self.block_actors.items():
      actor.setSolidColor(white)
      if actor.solid_visible or actor.edges_visible:
        actor.showSolid()
        actor.showEdges()
        
    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()
    
