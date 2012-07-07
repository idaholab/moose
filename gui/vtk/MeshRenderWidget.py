import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
from vtk.util.colors import peacock, tomato, red, white, black

from PeacockActor import PeacockActor
from ClippedActor import ClippedActor

import RendererFactory

class MeshRenderWidget(QtGui.QWidget):
  def __init__(self, tree_widget):
    QtGui.QWidget.__init__(self)
    self.tree_widget = tree_widget
    self.tree_widget.mesh_item_changed.connect(self.meshItemChanged)

    self.mesh_file_name = ''

    self.current_block_actors = {}
    self.current_sideset_actors = {}
    self.current_nodeset_actors = {}

    self.this_layout = QtGui.QVBoxLayout()
    self.setLayout(self.this_layout)

    self.vtkwidget = vtk.QVTKWidget2()

    self.renderer = vtk.vtkRenderer()
    self.renderer.SetBackground(0.2,0.2,0.2)
    self.renderer.SetBackground2(1,1,1)
    self.renderer.SetGradientBackground(1)
    self.renderer.ResetCamera()
    
    self.this_layout.addWidget(self.vtkwidget)
    self.vtkwidget.show()    

    self.vtkwidget.GetRenderWindow().AddRenderer(self.renderer)

    self.highlight_layout = QtGui.QHBoxLayout()

    self.view_mesh_checkbox = QtGui.QCheckBox('View Mesh')
    self.highlight_layout.addWidget(self.view_mesh_checkbox)

    self.highlight_block_label = QtGui.QLabel('Highlight Block:')
    self.highlight_block_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
    self.highlight_block_combo = QtGui.QComboBox()
    self.highlight_block_combo.currentIndexChanged[str].connect(self.showBlockSelected)
    self.highlight_layout.addWidget(self.highlight_block_label)
    self.highlight_layout.addWidget(self.highlight_block_combo)

    self.highlight_sideset_label = QtGui.QLabel('Highlight Sideset:')
    self.highlight_sideset_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
    self.highlight_sideset_combo = QtGui.QComboBox()
    self.highlight_sideset_combo.currentIndexChanged[str].connect(self.showSidesetSelected)
    self.highlight_layout.addWidget(self.highlight_sideset_label)
    self.highlight_layout.addWidget(self.highlight_sideset_combo)

    self.highlight_nodeset_label = QtGui.QLabel('Highlight Nodeset:')
    self.highlight_nodeset_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
    self.highlight_nodeset_combo = QtGui.QComboBox()
    self.highlight_nodeset_combo.currentIndexChanged[str].connect(self.showNodesetSelected)
    self.highlight_layout.addWidget(self.highlight_nodeset_label)
    self.highlight_layout.addWidget(self.highlight_nodeset_combo)

    self.highlight_clear = QtGui.QPushButton('Clear Highlight')
    self.highlight_clear.setDisabled(True)
    self.highlight_clear.clicked.connect(self.clearHighlight)
    self.highlight_layout.addWidget(self.highlight_clear)
    
    self.this_layout.addLayout(self.highlight_layout)

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

  def clear(self):
    self.highlight_block_combo.clear()
    self.highlight_sideset_combo.clear()
    self.highlight_nodeset_combo.clear()

    for block_actor_name, block_actor in self.current_block_actors.items():
      block_actor.hide()
      
    for sideset_actor_name, sideset_actor in self.current_sideset_actors.items():
      sideset_actor.hide()
      
    for nodeset_actor_name, nodeset_actor in self.current_nodeset_actors.items():
      nodeset_actor.hide()
      
    self.current_block_actors = {}
    self.current_sideset_actors = {}
    self.current_nodeset_actors = {}

  def meshItemChanged(self, item):
    # Disconnect some actions while we fill stuff in
    self.highlight_block_combo.currentIndexChanged[str].disconnect(self.showBlockSelected)
    self.highlight_sideset_combo.currentIndexChanged[str].disconnect(self.showSidesetSelected)
    self.highlight_nodeset_combo.currentIndexChanged[str].disconnect(self.showNodesetSelected)
    
    self.clear()
    
    self.mesh_renderer = RendererFactory.getRenderer(self, item.table_data)

    if self.mesh_renderer:
      self.show()
    else:
      self.hide()
      return 

    self.current_block_actors = self.mesh_renderer.block_actors
    self.current_sideset_actors = self.mesh_renderer.sideset_actors
    self.current_nodeset_actors = self.mesh_renderer.nodeset_actors

    for block_actor_name, block_actor in self.current_block_actors.items():
      block_actor.show()  
      block_actor.showEdges()
      
    sorted_block_names = set()
    for block_actor_name, block_actor in self.current_block_actors.items():
      sorted_block_names.add(int(block_actor_name))
    self.highlight_block_combo.addItem('')
    for block_actor_name in sorted_block_names:
      self.highlight_block_combo.addItem(str(block_actor_name))

    sorted_sideset_names = set()
    for sideset_actor_name, sideset_actor in self.current_sideset_actors.items():
      sorted_sideset_names.add(sideset_actor_name)
    self.highlight_sideset_combo.addItem('')
    for sideset_actor_name in sorted_sideset_names:
      self.highlight_sideset_combo.addItem(sideset_actor_name)

    sorted_nodeset_names = set()
    for nodeset_actor_name, nodeset_actor in self.current_nodeset_actors.items():
      sorted_nodeset_names.add(nodeset_actor_name)
    self.highlight_nodeset_combo.addItem('')
    for nodeset_actor_name in sorted_nodeset_names:
      self.highlight_nodeset_combo.addItem(nodeset_actor_name)


    self.highlight_block_combo.currentIndexChanged[str].connect(self.showBlockSelected)
    self.highlight_sideset_combo.currentIndexChanged[str].connect(self.showSidesetSelected)
    self.highlight_nodeset_combo.currentIndexChanged[str].connect(self.showNodesetSelected)

    self.setBounds()
    
     # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()    
   
    
  def setBounds(self):
    for actor_name, actor in self.current_block_actors.items():
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
      self.swapActors(self.current_block_actors, self.mesh_renderer.clipped_block_actors)
      self.current_block_actors = self.mesh_renderer.clipped_block_actors
      self.swapActors(self.current_sideset_actors, self.mesh_renderer.clipped_sideset_actors)
      self.current_sideset_actors = self.mesh_renderer.clipped_sideset_actors
      self.swapActors(self.current_nodeset_actors, self.mesh_renderer.clipped_nodeset_actors)
      self.current_nodeset_actors = self.mesh_renderer.clipped_nodeset_actors
    else:
      self.swapActors(self.current_block_actors, self.mesh_renderer.block_actors)
      self.current_block_actors = self.mesh_renderer.block_actors
      self.swapActors(self.current_sideset_actors, self.mesh_renderer.sideset_actors)
      self.current_sideset_actors = self.mesh_renderer.sideset_actors
      self.swapActors(self.current_nodeset_actors, self.mesh_renderer.nodeset_actors)
      self.current_nodeset_actors = self.mesh_renderer.nodeset_actors

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

  def showBlockSelected(self, block_name):
    self.highlight_sideset_combo.setCurrentIndex(0)
    self.highlight_nodeset_combo.setCurrentIndex(0)
    if block_name != '':
      self.highlightBlock(str(block_name))

  def showSidesetSelected(self, sideset_name):
    self.highlight_block_combo.setCurrentIndex(0)
    self.highlight_nodeset_combo.setCurrentIndex(0)
    if sideset_name != '':
      self.highlightBoundary(str(sideset_name))

  def showNodesetSelected(self, nodeset_name):
    self.highlight_block_combo.setCurrentIndex(0)
    self.highlight_sideset_combo.setCurrentIndex(0)
    if nodeset_name != '':
      self.highlightNodeset(str(nodeset_name))

  def highlightBoundary(self, boundary):
    self.highlight_clear.setDisabled(False)
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
    
  def highlightNodeset(self, boundary):
    self.highlight_clear.setDisabled(False)
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
      if the_boundary in self.current_nodeset_actors:
        self.current_nodeset_actors[the_boundary].show()
    
    self.vtkwidget.updateGL()

  def highlightBlock(self, block):
    self.highlight_clear.setDisabled(False)
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
    self.highlight_block_combo.setCurrentIndex(0)
    self.highlight_sideset_combo.setCurrentIndex(0)
    self.highlight_nodeset_combo.setCurrentIndex(0)
    self.highlight_clear.setDisabled(True)
    
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
