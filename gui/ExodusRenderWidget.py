import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
from vtk.util.colors import peacock, tomato, red, white, black

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class ExodusMap:
  # These are the blocks from the multiblockdataset that correspond to each item
  element_vtk_block = 0
  sideset_vtk_block = 4
  nodeset_vtk_block = 7

class PeacockActor:
  def __init__(self, renderer):
    self.renderer = renderer
    
    self.visible = False
    self.edges_visible = False
    self.solid = True
    self.color = white
    
  ''' Sync view options.  The view options for self will match the passed in object '''
  def sync(self, other_actor):
    if other_actor.visible:
      self.show()
    else:
      self.hide()

    if other_actor.edges_visible:
      self.showEdges()
    else:
      self.hideEdges()

    if other_actor.solid:
      self.goSolid()
    else:
      self.goWireframe()

    self.setColor(other_actor.color)
    
  def getBounds(self):
    raise NotImplementedError    

  def show(self):
    self.visible = True
    self._show()

  def hide(self):
    self.visible = False
    self._hide()

  def showEdges(self):
    self.edges_visible = True
    self._showEdges()
    
  def hideEdges(self):
    self.edges_visible = False
    self._hideEdges()

  def goSolid(self):
    self.solid = True
    self._goSolid()
    
  def goWireframe(self):
    self.solid = False
    self._goWireframe()

  def setColor(self, color):
    self.color = color
    self._setColor(color)

  def _show(self):
    raise NotImplementedError
  
  def _hide(self):
    raise NotImplementedError

  def _showEdges(self):
    raise NotImplementedError
    
  def _hideEdges(self):
    raise NotImplementedError

  def _goSolid(self):
    raise NotImplementedError
    
  def _goWireframe(self):
    raise NotImplementedError

  def _setColor(self, color):
    raise NotImplementedError


class ExodusActor(PeacockActor):
  def __init__(self, renderer, data, type, index):
    PeacockActor.__init__(self, renderer)
    self.data = data
    self.type = type
    self.index = index

    self.solid_visible = False
    self.edges_visible = False
    
    self.mesh = data.GetBlock(type).GetBlock(index)
    
    self.geom = vtk.vtkDataSetSurfaceFilter()
    self.geom.SetInput(self.mesh)
    self.geom.Update()
    
    self.mapper = vtk.vtkDataSetMapper()
    self.mapper.SetInput(self.mesh)
    
    self.actor = vtk.vtkActor()
    self.actor.SetMapper(self.mapper)
    self.actor.GetProperty().SetPointSize(4)
    self.actor.GetProperty().SetEdgeColor(0,0,0)
    self.actor.GetProperty().SetAmbient(0.3);

  def getBounds(self):
    return self.actor.GetBounds()

  def _show(self):
    self.renderer.AddActor(self.actor)

  def _hide(self):
    self.renderer.RemoveActor(self.actor)

  def _showEdges(self):
    self.actor.GetProperty().EdgeVisibilityOn()
    
  def _hideEdges(self):
    self.actor.GetProperty().EdgeVisibilityOff()

  def _goSolid(self):
    self.actor.GetProperty().SetRepresentationToSurface()
    
  def _goWireframe(self):
    self.actor.GetProperty().SetRepresentationToWireframe()

  def _setColor(self, color):
    self.actor.GetProperty().SetColor(color)

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

  def getBounds(self):
    return self.original_actor.getBounds()

  def movePlane(self, distance):
    self.clipper.SetValue(distance)
    self.cutter.SetValue(0, distance)
    self.stripper.Update()
    self.cut_poly.SetPoints(self.stripper.GetOutput().GetPoints())
    self.cut_poly.SetPolys(self.stripper.GetOutput().GetLines())
    self.cut_mapper.Update()
  
  def _show(self):
    self.original_actor.renderer.AddActor(self.clip_actor)
    self.original_actor.renderer.AddActor(self.cut_actor)

  def _hide(self):
    self.original_actor.renderer.RemoveActor(self.clip_actor)
    self.original_actor.renderer.RemoveActor(self.cut_actor)

  def _showEdges(self):
    self.clip_actor.GetProperty().EdgeVisibilityOn()
#    self.cut_actor.GetProperty().EdgeVisibilityOn()
    
  def _hideEdges(self):
    self.clip_actor.GetProperty().EdgeVisibilityOff()
#    self.cut_actor.GetProperty().EdgeVisibilityOff()

  def _goSolid(self):
    self.clip_actor.GetProperty().SetRepresentationToSurface()
    self.original_actor.renderer.AddActor(self.cut_actor)
    
#    self.cut_actor.GetProperty().SetRepresentationToSurface()
    
  def _goWireframe(self):
    self.clip_actor.GetProperty().SetRepresentationToWireframe()
    self.original_actor.renderer.RemoveActor(self.cut_actor)
#    self.cut_actor.GetProperty().SetRepresentationToWireframe()

  def _setColor(self, color):
    self.clip_actor.GetProperty().SetColor(color)
    self.cut_actor.GetProperty().SetColor(color)

class ExodusRenderWidget(QtGui.QWidget):
  def __init__(self):
    QtGui.QWidget.__init__(self)
    self.this_layout = QtGui.QVBoxLayout()
    self.setMinimumWidth(700)
    self.setLayout(self.this_layout)

    self.vtkwidget = vtk.QVTKWidget2()

    self.renderer = vtk.vtkRenderer()
    self.renderer.SetBackground(0.2,0.2,0.2)
    self.renderer.SetBackground2(1,1,1)
    self.renderer.SetGradientBackground(1)
    self.renderer.ResetCamera()
    
    self.this_layout.addWidget(self.vtkwidget)
    self.vtkwidget.show()    

    self.current_actors = []

    self.vtkwidget.GetRenderWindow().AddRenderer(self.renderer)

    self.plane = vtk.vtkPlane()
    self.plane.SetOrigin(0, 0, 0)
    self.plane.SetNormal(1, 0, 0)

    self.clip_groupbox = QtGui.QGroupBox("Clip")
    self.clip_groupbox.setCheckable(True)
    self.clip_groupbox.setChecked(False)
    self.clip_groupbox.setMaximumHeight(70)
    self.clip_groupbox.toggled[bool].connect(self._clippingToggled)
    clip_layout = QtGui.QHBoxLayout()
    
    self.clip_plane_combobox = QtGui.QComboBox()
    self.clip_plane_combobox.addItem('x')
    self.clip_plane_combobox.addItem('y')
    self.clip_plane_combobox.addItem('z')
    self.clip_plane_combobox.currentIndexChanged[str].connect(self._clipNormalChanged)

    clip_layout.addWidget(self.clip_plane_combobox)
    
    self.clip_plane_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
    self.clip_plane_slider.setRange(0, 100)
    self.clip_plane_slider.setSliderPosition(50)
    self.clip_plane_slider.sliderMoved[int].connect(self._clipSliderMoved)
    clip_layout.addWidget(self.clip_plane_slider)
#     vbox->addStretch(1);
    self.clip_groupbox.setLayout(clip_layout)

    self.this_layout.addWidget(self.clip_groupbox)

    self.bounds = {}
    self.bounds['x'] = [0.0, 0.0]
    self.bounds['y'] = [0.0, 0.0]
    self.bounds['z'] = [0.0, 0.0]

#    self.draw_edges_checkbox = QtGui.QCheckBox("View Mesh")

#    self.left_controls_layout.addWidget(self.draw_edges_checkbox)

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
      nodeset_id = reader.GetObjectId(vtk.vtkExodusIIReader.NODE_SET,i)
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

    self.all_actors = []

    self.sideset_actors = {}
    self.clipped_sideset_actors = {}
    
    self.current_sideset_actors = self.sideset_actors
    for i in xrange(num_sidesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.sideset_vtk_block, i)
      actor.setColor(red)
      self.sideset_actors[str(self.sidesets[i])] = actor
      self.all_actors.append(actor)

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_sideset_actors[str(self.sidesets[i])] = clipped_actor
      self.all_actors.append(clipped_actor)
      
      name = reader.GetObjectName(vtk.vtkExodusIIReader.SIDE_SET,i).split(' ')
      if 'Unnamed' not in name:
        self.sideset_actors[name[0]] = actor
        self.clipped_sideset_actors[name[0]] = clipped_actor

    self.nodeset_actors = {}
    self.clipped_nodeset_actors = {}

    self.current_nodeset_actors = self.nodeset_actors
    for i in xrange(num_nodesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.nodeset_vtk_block, i)
      actor.setColor(red)
      self.nodeset_actors[str(self.nodesets[i])] = actor
      self.all_actors.append(actor)

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_nodeset_actors[str(self.nodesets[i])] = clipped_actor
      self.all_actors.append(clipped_actor)

      name = reader.GetObjectName(vtk.vtkExodusIIReader.NODE_SET,i).split(' ')
      if 'Unnamed' not in name:
        self.nodeset_actors[name[0]] = actor
        self.clipped_nodeset_actors[name[0]] = clipped_actor

    self.block_actors = {}
    self.clipped_block_actors = {}

    self.current_block_actors = self.block_actors
    for i in xrange(num_blocks):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.element_vtk_block, i)
      self.block_actors[str(self.blocks[i])] = actor
      self.all_actors.append(actor)

      actor.show()
      actor.showEdges()

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_block_actors[str(self.blocks[i])] = clipped_actor
      self.all_actors.append(clipped_actor)
      
      name = reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' ')
      if 'Unnamed' not in name:
        self.block_actors[name[0]] = actor
        self.clipped_block_actors[name[0]] = clipped_actor

    self.setBounds()
    
    # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()

  def setBounds(self):
    for actor in self.all_actors:
      current_bounds = actor.getBounds()
      self.bounds['x'][0] = min(self.bounds['x'][0], current_bounds[0])
      self.bounds['x'][1] = max(self.bounds['x'][1], current_bounds[1])

      self.bounds['y'][0] = min(self.bounds['y'][0], current_bounds[2])
      self.bounds['y'][1] = max(self.bounds['y'][1], current_bounds[3])

      self.bounds['z'][0] = min(self.bounds['z'][0], current_bounds[4])
      self.bounds['z'][1] = max(self.bounds['z'][1], current_bounds[5])

  def swapActors(self, current, new):
    for old_name, old_actor in current.items():
      new[old_name].sync(old_actor)
      old_actor.hide()

  def _clippingToggled(self, value):
    if value:
      self.swapActors(self.current_block_actors, self.clipped_block_actors)
      self.current_block_actors = self.clipped_block_actors
      self.swapActors(self.current_sideset_actors, self.clipped_sideset_actors)
      self.current_sideset_actors = self.clipped_sideset_actors
      self.swapActors(self.current_nodeset_actors, self.clipped_nodeset_actors)
      self.current_nodeset_actors = self.clipped_nodeset_actors
    else:
      self.swapActors(self.current_block_actors, self.block_actors)
      self.current_block_actors = self.block_actors
      self.swapActors(self.current_sideset_actors, self.sideset_actors)
      self.current_sideset_actors = self.sideset_actors
      self.swapActors(self.current_nodeset_actors, self.nodeset_actors)
      self.current_nodeset_actors = self.nodeset_actors

    self.vtkwidget.updateGL()
    
  def _clipNormalChanged(self, value):
    if value == 'x':
      self.plane.SetNormal(1, 0, 0)
    elif value == 'y':
      self.plane.SetNormal(0, 1, 0)
    else:
      self.plane.SetNormal(0, 0, 1)

    self.clip_plane_slider.setSliderPosition(50)
    self._clipSliderMoved(50)
  
  def _clipSliderMoved(self, value):
    direction = str(self.clip_plane_combobox.currentText())
    step_size = (self.bounds[direction][1] - self.bounds[direction][0])/100.0
    steps = value-50
    distance = float(steps)*step_size
    
    for actor_name, actor in self.current_sideset_actors.items():
      actor.movePlane(distance)

    for actor_name, actor in self.current_nodeset_actors.items():
      actor.movePlane(distance)

    for actor_name, actor in self.current_block_actors.items():
      actor.movePlane(distance)

    self.vtkwidget.updateGL()    
    
    
  def highlightBoundary(self, boundary):
    # Turn off all sidesets
    for actor_name, actor in self.current_sideset_actors.items():
      actor.hide()

    # Turn off all nodesets
    for actor_name, actor in self.current_nodeset_actors.items():
      actor.hide()

    # Turn solids to only edges... but only if they are visible
    for actor_name, actor in self.current_block_actors.items():
      actor.setColor(black)
      actor.goWireframe()

    boundaries = boundary.strip("'").split(' ')
    for the_boundary in boundaries:
      if the_boundary in self.current_sideset_actors:
        self.current_sideset_actors[the_boundary].show()
      elif the_boundary in self.current_nodeset_actors:
        self.current_nodeset_actors[the_boundary].show()
    
    self.vtkwidget.updateGL()

  def highlightBlock(self, block):
    # Turn off all sidesets
    for actor_name, actor in self.current_sideset_actors.items():
      actor.hide()

    # Turn off all nodesets
    for actor_name, actor in self.current_nodeset_actors.items():
      actor.hide()

    # Turn solids to only edges...
    for actor_name, actor in self.current_block_actors.items():
      actor.setColor(black)
      actor.goWireframe()

    blocks = block.strip("'").split(' ')
    for the_block in blocks:
      if the_block in self.current_block_actors:
        self.current_block_actors[the_block].setColor(red)
        self.current_block_actors[the_block].goSolid()
    
    self.vtkwidget.updateGL()

  def clearHighlight(self):
    # Turn off all sidesets
    for actor_name, actor in self.current_sideset_actors.items():
      actor.hide()

    # Turn off all nodesets
    for actor_name, actor in self.current_nodeset_actors.items():
      actor.hide()

    # Show solids and edges - but only if something is visible
    for actor_name, actor in self.current_block_actors.items():
      actor.setColor(white)
      actor.goSolid()
        
    self.vtkwidget.updateGL()
    
