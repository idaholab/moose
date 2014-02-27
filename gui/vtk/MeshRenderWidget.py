import os, sys, getopt

try:
    from PyQt4 import QtCore, QtGui
    QtCore.Signal = QtCore.pyqtSignal
    QtCore.Slot = QtCore.pyqtSlot
except ImportError:
    try:
        from PySide import QtCore, QtGui
    except ImportError:
        raise ImportError("Cannot load either PyQt or PySide")

import vtk
from vtk.util.colors import peacock, tomato, red, white, black
from vtk.qt4.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor

from PeacockActor import PeacockActor
from ClippedActor import ClippedActor

import RendererFactory

class MeshRenderWidget(QtGui.QWidget):
  def __init__(self, tree_widget):
    QtGui.QWidget.__init__(self)
    self.tree_widget = tree_widget
    self.tree_widget.mesh_item_changed.connect(self.meshItemChanged)

    self.mesh_file_name = ''

    self.mesh_renderer = None

    self.current_block_actors = {}
    self.current_sideset_actors = {}
    self.current_nodeset_actors = {}

    self.this_layout = QtGui.QVBoxLayout()
    self.setLayout(self.this_layout)

    self.vtkwidget = QVTKRenderWindowInteractor(self)

    self.renderer = vtk.vtkRenderer()
    self.renderer.SetBackground(0.2,0.2,0.2)
    self.renderer.SetBackground2(1,1,1)
    self.renderer.SetGradientBackground(1)
    self.renderer.ResetCamera()

    self.this_layout.addWidget(self.vtkwidget)
    self.this_layout.setStretchFactor(self.vtkwidget, 10)

    self.vtkwidget.setMinimumHeight(300)

    self.vtkwidget.GetRenderWindow().AddRenderer(self.renderer)
    self.interactor = self.vtkwidget.GetRenderWindow().GetInteractor()

    self.interactor.SetInteractorStyle(vtk.vtkInteractorStyleTrackballCamera())

    self.vtkwidget.Initialize()
    self.vtkwidget.Start()

    self.controls_layout = QtGui.QHBoxLayout()

    self.left_controls_layout = QtGui.QVBoxLayout()

    self.block_view_group_box = QtGui.QGroupBox('Show Blocks')
    self.block_view_group_box.setMaximumWidth(150)
#    self.block_view_group_box.setMaximumHeight(200)

    self.block_view_layout = QtGui.QVBoxLayout()
    self.block_view_list = QtGui.QListView()
    self.block_view_model = QtGui.QStandardItemModel()
    self.block_view_model.itemChanged.connect(self._blockViewItemChanged)
    self.block_view_list.setModel(self.block_view_model)
    self.block_view_layout.addWidget(self.block_view_list)

    self.block_view_group_box.setLayout(self.block_view_layout)
    self.left_controls_layout.addWidget(self.block_view_group_box)
    self.controls_layout.addLayout(self.left_controls_layout)


    self.right_controls_layout = QtGui.QVBoxLayout()
    self.controls_layout.addLayout(self.right_controls_layout)

    self.view_mesh_checkbox = QtGui.QCheckBox('View Mesh')
    self.view_mesh_checkbox.setToolTip('Toggle viewing of mesh elements')
    self.view_mesh_checkbox.setCheckState(QtCore.Qt.Checked)
    self.view_mesh_checkbox.stateChanged.connect(self.viewMeshCheckboxChanged)
    self.right_controls_layout.addWidget(self.view_mesh_checkbox)

    self.highlight_group_box = QtGui.QGroupBox('Highlight')
    self.highlight_group_box.setMaximumHeight(70)
    self.highlight_group_box.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding, QtGui.QSizePolicy.Fixed)
#    self.highlight_group_box.setMaximumWidth(200)
    self.highlight_layout = QtGui.QHBoxLayout()
    self.highlight_group_box.setLayout(self.highlight_layout)
    self.right_controls_layout.addWidget(self.highlight_group_box)

    self.highlight_block_label = QtGui.QLabel('Block:')
    self.highlight_block_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
    self.highlight_block_combo = QtGui.QComboBox()
#    self.highlight_block_combo.setMaximumWidth(50)
    self.highlight_block_combo.setSizeAdjustPolicy(QtGui.QComboBox.AdjustToMinimumContentsLength)
    self.highlight_block_combo.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding, QtGui.QSizePolicy.Fixed)
    self.highlight_block_combo.setToolTip('Highlight a block in the mesh')
    self.highlight_block_combo.currentIndexChanged[str].connect(self.showBlockSelected)
    self.highlight_layout.addWidget(self.highlight_block_label)
    self.highlight_layout.addWidget(self.highlight_block_combo)

    self.highlight_sideset_label = QtGui.QLabel('Sideset:')
    self.highlight_sideset_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
    self.highlight_sideset_combo = QtGui.QComboBox()
#    self.highlight_sideset_combo.setMaximumWidth(50)
    self.highlight_sideset_combo.setSizeAdjustPolicy(QtGui.QComboBox.AdjustToMinimumContentsLength)
    self.highlight_sideset_combo.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding, QtGui.QSizePolicy.Fixed)
    self.highlight_sideset_combo.setToolTip('Highlight a sideset in the mesh')
    self.highlight_sideset_combo.currentIndexChanged[str].connect(self.showSidesetSelected)
    self.highlight_layout.addWidget(self.highlight_sideset_label)
    self.highlight_layout.addWidget(self.highlight_sideset_combo)

    self.highlight_nodeset_label = QtGui.QLabel('Nodeset:')
    self.highlight_nodeset_label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
    self.highlight_nodeset_combo = QtGui.QComboBox()
#    self.highlight_nodeset_combo.setMaximumWidth(50)
    self.highlight_nodeset_combo.setSizeAdjustPolicy(QtGui.QComboBox.AdjustToMinimumContentsLength)
    self.highlight_nodeset_combo.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding, QtGui.QSizePolicy.Fixed)
    self.highlight_nodeset_combo.setToolTip('Highlight a nodeset in the mesh')
    self.highlight_nodeset_combo.currentIndexChanged[str].connect(self.showNodesetSelected)
    self.highlight_layout.addWidget(self.highlight_nodeset_label)
    self.highlight_layout.addWidget(self.highlight_nodeset_combo)

    self.highlight_clear = QtGui.QPushButton('Clear')
    self.highlight_clear.setToolTip('Clear highlighting')
    self.highlight_clear.setDisabled(True)
    self.highlight_clear.clicked.connect(self.clearHighlight)
    self.highlight_layout.addWidget(self.highlight_clear)

    self.plane = vtk.vtkPlane()
    self.plane.SetOrigin(0, 0, 0)
    self.plane.SetNormal(1, 0, 0)

    self.clip_groupbox = QtGui.QGroupBox("Clip")
    self.clip_groupbox.setToolTip('Toggle clip mode to slice the mesh open along a plane')
    self.clip_groupbox.setCheckable(True)
    self.clip_groupbox.setChecked(False)
    self.clip_groupbox.setMaximumHeight(70)
    self.clip_groupbox.toggled[bool].connect(self._clippingToggled)
    clip_layout = QtGui.QHBoxLayout()

    self.clip_plane_combobox = QtGui.QComboBox()
    self.clip_plane_combobox.setToolTip('Direction of the normal for the clip plane')
    self.clip_plane_combobox.addItem('x')
    self.clip_plane_combobox.addItem('y')
    self.clip_plane_combobox.addItem('z')
    self.clip_plane_combobox.currentIndexChanged[str].connect(self._clipNormalChanged)

    clip_layout.addWidget(self.clip_plane_combobox)

    self.clip_plane_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
    self.clip_plane_slider.setToolTip('Slide to change plane position')
    self.clip_plane_slider.setRange(0, 100)
    self.clip_plane_slider.setSliderPosition(50)
    self.clip_plane_slider.sliderMoved[int].connect(self._clipSliderMoved)
    clip_layout.addWidget(self.clip_plane_slider)
#     vbox->addStretch(1);
    self.clip_groupbox.setLayout(clip_layout)

    self.right_controls_layout.addWidget(self.clip_groupbox)

    self.this_layout.addLayout(self.controls_layout)
    self.this_layout.setStretchFactor(self.controls_layout, 1)

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
    if self.mesh_renderer:
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

    self.block_view_model.clear()
    for block in self.mesh_renderer.blocks:
      block_display_name = str(block)
      if block in self.mesh_renderer.block_id_to_name:
        block_display_name += ' : ' + self.mesh_renderer.block_id_to_name[block]

      item = QtGui.QStandardItem(str(block_display_name))
      item.exodus_block = block
      item.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
      item.setCheckState(QtCore.Qt.Checked)
      self.block_view_model.appendRow(item)

    for block_actor_name, block_actor in self.current_block_actors.items():
      block_actor.show()
      block_actor.showEdges()

    block_names = []
    for block_actor_id, block_actor in self.current_block_actors.items():
      name = block_actor_id.strip(' ')
      if int(name) in self.mesh_renderer.block_id_to_name:
        name += ' : ' + self.mesh_renderer.block_id_to_name[int(name)]

      block_names.append(name)

    self.highlight_block_combo.addItem('')
    for block_actor_name in sorted(block_names, key=lambda name: int(name.split(' ')[0])):
      self.highlight_block_combo.addItem(str(block_actor_name))

    sideset_names = []
    for sideset_actor_id, sideset_actor in self.current_sideset_actors.items():
      sideset_actor.setColor(red)

      name = sideset_actor_id.strip(' ')
      if int(name) in self.mesh_renderer.sideset_id_to_name:
        name += ' : ' + self.mesh_renderer.sideset_id_to_name[int(name)]

      sideset_names.append(name)

    self.highlight_sideset_combo.addItem('')
    for sideset_actor_name in sorted(sideset_names, key=lambda name: int(name.split(' ')[0])):
      self.highlight_sideset_combo.addItem(sideset_actor_name)

    nodeset_names = []
    for nodeset_actor_id, nodeset_actor in self.current_nodeset_actors.items():
      nodeset_actor.setColor(red)

      name = nodeset_actor_id.strip(' ')
      if int(name) in self.mesh_renderer.nodeset_id_to_name:
        name += ' : ' + self.mesh_renderer.nodeset_id_to_name[int(name)]

      nodeset_names.append(name)

    self.highlight_nodeset_combo.addItem('')
    for nodeset_actor_name in sorted(nodeset_names, key=lambda name: int(name.split(' ')[0])):
      self.highlight_nodeset_combo.addItem(nodeset_actor_name)


    self.highlight_block_combo.currentIndexChanged[str].connect(self.showBlockSelected)
    self.highlight_sideset_combo.currentIndexChanged[str].connect(self.showSidesetSelected)
    self.highlight_nodeset_combo.currentIndexChanged[str].connect(self.showNodesetSelected)

    self.setBounds()

     # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

    self.renderer.ResetCamera()
    self.vtkwidget.repaint()

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

  def _blockViewItemChanged(self, item):
    if item.checkState() == QtCore.Qt.Checked:
      self.current_block_actors[str(item.exodus_block)].show()
    else:
      self.current_block_actors[str(item.exodus_block)].hide()
    self.vtkwidget.repaint()

  def _clippingToggled(self, value):
    if value:
      self.swapActors(self.current_block_actors, self.mesh_renderer.clipped_block_actors)
      self.current_block_actors = self.mesh_renderer.clipped_block_actors
      self.swapActors(self.current_sideset_actors, self.mesh_renderer.clipped_sideset_actors)
      self.current_sideset_actors = self.mesh_renderer.clipped_sideset_actors
      self.swapActors(self.current_nodeset_actors, self.mesh_renderer.clipped_nodeset_actors)
      self.current_nodeset_actors = self.mesh_renderer.clipped_nodeset_actors

      self._clipNormalChanged(self.clip_plane_combobox.currentText())
    else:
      self.swapActors(self.current_block_actors, self.mesh_renderer.block_actors)
      self.current_block_actors = self.mesh_renderer.block_actors
      self.swapActors(self.current_sideset_actors, self.mesh_renderer.sideset_actors)
      self.current_sideset_actors = self.mesh_renderer.sideset_actors
      self.swapActors(self.current_nodeset_actors, self.mesh_renderer.nodeset_actors)
      self.current_nodeset_actors = self.mesh_renderer.nodeset_actors

    self.vtkwidget.repaint()

  def _clipNormalChanged(self, value):
    self.plane.SetOrigin(self.bounds['x'][0],
                         self.bounds['y'][0],
                         self.bounds['z'][0])
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
    steps = value
    distance = float(steps)*step_size
    position = self.bounds[direction][0] + distance

    old = self.plane.GetOrigin()
    self.plane.SetOrigin(position if direction == 'x' else old[0],
                         position if direction == 'y' else old[1],
                         position if direction == 'z' else old[2])

    for actor_name, actor in self.current_sideset_actors.items():
      actor.movePlane()

    for actor_name, actor in self.current_nodeset_actors.items():
      actor.movePlane()

    for actor_name, actor in self.current_block_actors.items():
      actor.movePlane()

    self.vtkwidget.repaint()

  def viewMeshCheckboxChanged(self, value):
    if value == QtCore.Qt.Checked:
      for actor_name, actor in self.current_sideset_actors.items():
        actor.showEdges()
      for actor_name, actor in self.current_nodeset_actors.items():
        actor.showEdges()
      for actor_name, actor in self.current_block_actors.items():
        actor.showEdges()
    else:
      for actor_name, actor in self.current_sideset_actors.items():
        actor.hideEdges()
      for actor_name, actor in self.current_nodeset_actors.items():
        actor.hideEdges()
      for actor_name, actor in self.current_block_actors.items():
        actor.hideEdges()
    self.vtkwidget.repaint()

  def clearBlockComboBox(self):
      self.highlight_block_combo.currentIndexChanged[str].disconnect(self.showBlockSelected)
      self.highlight_block_combo.setCurrentIndex(0)
      self.highlight_block_combo.currentIndexChanged[str].connect(self.showBlockSelected)

  def clearSidesetComboBox(self):
    self.highlight_sideset_combo.currentIndexChanged[str].disconnect(self.showSidesetSelected)
    self.highlight_sideset_combo.setCurrentIndex(0)
    self.highlight_sideset_combo.currentIndexChanged[str].connect(self.showSidesetSelected)

  def clearNodesetComboBox(self):
    self.highlight_nodeset_combo.currentIndexChanged[str].disconnect(self.showNodesetSelected)
    self.highlight_nodeset_combo.setCurrentIndex(0)
    self.highlight_nodeset_combo.currentIndexChanged[str].connect(self.showNodesetSelected)

  def showBlockSelected(self, block_name):
    if block_name != '':
      self.clearSidesetComboBox()
      self.clearNodesetComboBox()
      self.highlightBlock(str(block_name).split(' ')[0])
    else:
      self.clearActors()

  def showSidesetSelected(self, sideset_name):
    if sideset_name != '':
      self.clearBlockComboBox()
      self.clearNodesetComboBox()
      self.highlightBoundary(str(sideset_name).split(' ')[0])
    else:
      self.clearActors()

  def showNodesetSelected(self, nodeset_name):
    if nodeset_name != '':
      self.clearBlockComboBox()
      self.clearSidesetComboBox()
      self.highlightNodeset(str(nodeset_name).split(' ')[0])
    else:
      self.clearActors()

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
      elif the_boundary in self.mesh_renderer.name_to_sideset_id:
        self.current_sideset_actors[str(self.mesh_renderer.name_to_sideset_id[the_boundary])].show()
      elif the_boundary in self.mesh_renderer.name_to_nodeset_id:
        self.current_nodeset_actors[str(self.mesh_renderer.name_to_nodeset_id[the_boundary])].show()

    self.vtkwidget.repaint()

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
      elif the_boundary in self.mesh_renderer.name_to_nodeset_id:
        self.current_nodeset_actors[str(self.mesh_renderer.name_to_nodeset_id[the_boundary])].show()

    self.vtkwidget.repaint()

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
      elif the_block in self.mesh_renderer.name_to_block_id:
        self.current_block_actors[str(self.mesh_renderer.name_to_block_id[the_block])].setColor(red)
        self.current_block_actors[str(self.mesh_renderer.name_to_block_id[the_block])].goSolid()

    self.vtkwidget.repaint()

  def clearActors(self):
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

    self.vtkwidget.repaint()

  def clearHighlight(self):
    self.highlight_block_combo.setCurrentIndex(0)
    self.highlight_sideset_combo.setCurrentIndex(0)
    self.highlight_nodeset_combo.setCurrentIndex(0)
    self.highlight_clear.setDisabled(True)
    self.clearActors()
