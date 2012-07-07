import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
from vtk.util.colors import peacock, tomato, red, white, black

from PeacockActor import PeacockActor
from ExodusActor import ExodusActor
from ClippedActor import ClippedActor

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class ExodusMap:
  # These are the blocks from the multiblockdataset that correspond to each item
  element_vtk_block = 0
  sideset_vtk_block = 4
  nodeset_vtk_block = 7

class ExodusRenderWidget(QtGui.QWidget):
  def __init__(self):
    QtGui.QWidget.__init__(self)
    self.this_layout = QtGui.QVBoxLayout()
#    self.setMinimumWidth(700)
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
    
