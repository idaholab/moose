import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
import time
from ExodusResult import ExodusResult
import glob

pathname = os.path.dirname(sys.argv[0])        
pathname = os.path.abspath(pathname)

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class ExodusResultRenderWidget(QtGui.QWidget):
  def __init__(self, input_file_widget, execution_widget, qt_app):
    QtGui.QWidget.__init__(self)
    self.input_file_widget = input_file_widget
    self.qt_app = qt_app

    self.plane = vtk.vtkPlane()
    self.plane.SetOrigin(-1000, 0, 0)
    self.plane.SetNormal(1, 0, 0)

    self.exodus_result = None
    
    # The multiple (from adaptivity)
    self.exodus_results = []

    self.timestep_to_exodus_result = {}
    
    self.file_name = None

    # The multiple (from adaptivity) file names we know of
    self.file_names = []

    self.current_max_timestep = 0
    
    self.execution_widget = execution_widget
    self.execution_widget.run_started.connect(self._runStarted)
    self.execution_widget.run_stopped.connect(self._runStopped)
    self.execution_widget.timestep_begin.connect(self._timestepBegin)
    self.execution_widget.timestep_end.connect(self._timestepEnd)
    
    self.main_layout = QtGui.QVBoxLayout()
    self.setMinimumWidth(700)
    self.setLayout(self.main_layout)

    self.vtkwidget = vtk.QVTKWidget2()
    self.vtkwidget.setMinimumHeight(600)

    self.renderer = vtk.vtkRenderer()
    self.renderer.SetBackground(0.2,0.2,0.2)
    self.renderer.SetBackground2(1,1,1)
    self.renderer.SetGradientBackground(1)
    self.renderer.ResetCamera()
    
    self.main_layout.addWidget(self.vtkwidget)
    self.vtkwidget.show()

    self.vtkwidget.GetRenderWindow().AddRenderer(self.renderer)

    self.first = True

    self.exodus_result = None

    self.has_displacements = False
    self.current_displacement_magnitude = 1.0

    self.setupControls()

  def setupControls(self):
    self.controls_widget = QtGui.QWidget()
    self.controls_layout = QtGui.QHBoxLayout()
    self.main_layout.addLayout(self.controls_layout)

    self.leftest_controls_layout = QtGui.QVBoxLayout()
    self.left_controls_layout = QtGui.QVBoxLayout()
    self.right_controls_layout = QtGui.QVBoxLayout()

    self.block_view_group_box = QtGui.QGroupBox('Show Blocks')
    self.block_view_group_box.setMaximumWidth(200)
#    self.block_view_group_box.setMaximumHeight(200)
    
    self.block_view_layout = QtGui.QVBoxLayout()
    self.block_view_list = QtGui.QListView()
    self.block_view_model = QtGui.QStandardItemModel()
    self.block_view_model.itemChanged.connect(self._blockViewItemChanged)
    self.block_view_list.setModel(self.block_view_model)
    self.block_view_layout.addWidget(self.block_view_list)

    self.block_view_group_box.setLayout(self.block_view_layout)

    self.leftest_controls_layout.addWidget(self.block_view_group_box)

    self.controls_layout.addLayout(self.leftest_controls_layout)
    self.controls_layout.addLayout(self.left_controls_layout)
    self.controls_layout.addLayout(self.right_controls_layout)

    self.controls_layout.setStretchFactor(self.leftest_controls_layout,1.0)
    self.controls_layout.setStretchFactor(self.left_controls_layout,2.0)
    self.controls_layout.setStretchFactor(self.right_controls_layout,4.0)    

    self.automatic_update_checkbox = QtGui.QCheckBox("Automatically Update")
    self.automatic_update_checkbox.setToolTip('Toggle automattically reading new timesteps as they finish computing')
    self.automatic_update_checkbox.setCheckState(QtCore.Qt.Checked)
    self.automatically_update = True
    self.automatic_update_checkbox.stateChanged[int].connect(self._automaticUpdateChanged)
#    self.left_controls_layout.addWidget(self.automatic_update_checkbox)

    self.contour_groupbox = QtGui.QGroupBox("Contour")
    self.contour_groupbox.setMaximumHeight(70)
    self.contour_layout = QtGui.QHBoxLayout()
    self.contour_groupbox.setLayout(self.contour_layout)
    self.contour_label = QtGui.QLabel("Contour:")
    self.variable_contour = QtGui.QComboBox()
    self.variable_contour.setToolTip('Which variable to color by')
    self.variable_contour.currentIndexChanged[str].connect(self._contourVariableSelected)
#    self.contour_layout.addWidget(self.contour_label, alignment=QtCore.Qt.AlignRight)
    self.contour_layout.addWidget(self.variable_contour, alignment=QtCore.Qt.AlignLeft)

    self.component_layout = QtGui.QHBoxLayout()
    self.component_label = QtGui.QLabel("Component:")
    self.variable_component = QtGui.QComboBox()
    self.variable_component.setToolTip('If the variable is a vector this selects what component of that vector (or the Magnitude) to color by') 
    self.variable_component.currentIndexChanged[str].connect(self._variableComponentSelected)
#    self.component_layout.addWidget(self.component_label, alignment=QtCore.Qt.AlignRight)
    self.component_layout.addWidget(self.variable_component, alignment=QtCore.Qt.AlignLeft)
    self.contour_layout.addLayout(self.component_layout)

    self.left_controls_layout.addWidget(self.contour_groupbox)

    self.displace_groupbox = QtGui.QGroupBox("Displace")
    self.displace_groupbox.setCheckable(True)
    self.displace_groupbox.setChecked(True)
    self.displace_groupbox.setDisabled(True)
    self.displace_groupbox.setMaximumHeight(70)
    self.displace_groupbox.toggled[bool].connect(self._displaceToggled)
    self.displace_layout = QtGui.QHBoxLayout()
    self.displace_layout.setSpacing(0)
    self.displace_groupbox.setLayout(self.displace_layout)

    self.displace_magnitude_label = QtGui.QLabel("Multiplier: ")
    self.displace_magnitude_text = QtGui.QLineEdit("1.0")
    self.displace_magnitude_text.setMaximumWidth(50)
    self.displace_magnitude_text.returnPressed.connect(self._displaceMagnitudeTextReturn)

    self.displace_layout.addWidget(self.displace_magnitude_label, alignment=QtCore.Qt.AlignRight)
    self.displace_layout.addWidget(self.displace_magnitude_text, alignment=QtCore.Qt.AlignLeft)

    self.left_controls_layout.addWidget(self.displace_groupbox)

    self.reset_layout = QtGui.QHBoxLayout()
    self.draw_edges_checkbox = QtGui.QCheckBox("View Mesh")
    self.draw_edges_checkbox.setToolTip('Show mesh elements')
    self.draw_edges_checkbox.stateChanged[int].connect(self._drawEdgesChanged)
    self.reset_layout.addWidget(self.draw_edges_checkbox, alignment=QtCore.Qt.AlignHCenter)

    self.reset_button = QtGui.QPushButton('Reset View')
    self.reset_button.setMaximumWidth(100)
    self.reset_button.setToolTip('Recenter the camera on the current result')
    self.reset_button.clicked.connect(self._resetView)
    self.reset_layout.addWidget(self.reset_button, alignment=QtCore.Qt.AlignHCenter)

    self.left_controls_layout.addLayout(self.reset_layout)

    self.beginning_button = QtGui.QToolButton()
    self.beginning_button.setToolTip('Go to first timestep')
    self.beginning_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrFirst32.png'))
    self.beginning_button.clicked.connect(self._beginningClicked)

    self.back_button = QtGui.QToolButton()
    self.back_button.setToolTip('Previous timestep')
    self.back_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrBack32.png'))
    self.back_button.clicked.connect(self._backClicked)

    self.play_button = QtGui.QToolButton()
    self.play_button.setToolTip('Play through the currently computed timesteps')
    self.play_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrPlay32.png'))
    self.play_button.clicked.connect(self._playClicked)

    self.pause_button = QtGui.QToolButton()
    self.pause_button.setToolTip('If playing this will pause playback')
    self.pause_button.setDisabled(True)
    self.pause_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrPause32.png'))
    self.pause_button.clicked.connect(self._pauseClicked)

    self.forward_button = QtGui.QToolButton()
    self.forward_button.setToolTip('Next timestep')
    self.forward_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrForward32.png'))
    self.forward_button.clicked.connect(self._forwardClicked)

    self.last_button = QtGui.QToolButton()
    self.last_button.setToolTip('Go to last timestep')
    self.last_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrLast32.png'))
    self.last_button.clicked.connect(self._lastClicked)

    self.loop_button = QtGui.QToolButton()
    self.loop_button.setToolTip('Toggle replaying all timesteps when the end is reached during playback.  Note that as new timesteps finish they will automatically be picked up and added to the end of the sequence.')
    self.loop_button.setCheckable(True)
    self.loop_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrLoop24.png'))
    self.loop_button.toggled.connect(self._loopClicked)

    self.currently_looping = False

    self.time_slider_label = QtGui.QLabel("Timestep:")
    self.time_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
    self.time_slider.setToolTip('Slide to select a timestep to display')
    self.time_slider.setMaximumWidth(600)
    
    self.time_slider.valueChanged.connect(self._timeSliderChanged)
    self.time_slider.sliderReleased.connect(self._timeSliderReleased)
    
    self.time_slider_textbox = QtGui.QLineEdit()
    self.time_slider_textbox.setToolTip('Enter a number and press Enter to go to that timestep')
    self.time_slider_textbox.setMaximumWidth(30)
    self.time_slider_textbox.returnPressed.connect(self._sliderTextboxReturn)

    self.time_groupbox = QtGui.QGroupBox("Time")
    self.time_groupbox.setMaximumHeight(70)
    
    self.time_layout = QtGui.QHBoxLayout()
    self.time_layout.addWidget(self.beginning_button)
    self.time_layout.addWidget(self.back_button)
    self.time_layout.addWidget(self.play_button)
    self.time_layout.addWidget(self.pause_button)
    self.time_layout.addWidget(self.forward_button)
    self.time_layout.addWidget(self.last_button)
    self.time_layout.addWidget(self.loop_button)
    self.time_layout.addWidget(self.time_slider_label, alignment=QtCore.Qt.AlignRight)
    self.time_layout.addWidget(self.time_slider)
    self.time_layout.addWidget(self.time_slider_textbox, alignment=QtCore.Qt.AlignLeft)

    self.time_groupbox.setLayout(self.time_layout)

    self.right_controls_layout.addWidget(self.time_groupbox)

    self.clip_groupbox = QtGui.QGroupBox("Clip")
    self.clip_groupbox.setToolTip('Toggle clipping mode where the solution can be sliced open')
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
    self.clip_plane_slider.sliderReleased.connect(self._clipSliderReleased)
    self.clip_plane_slider.sliderMoved[int].connect(self._clipSliderMoved)
    clip_layout.addWidget(self.clip_plane_slider)
#     vbox->addStretch(1);
    self.clip_groupbox.setLayout(clip_layout)

    self.right_controls_layout.addWidget(self.clip_groupbox)


  def _updateControls(self):
    self.old_contour = self.variable_contour.currentText()
    self.variable_contour.clear()
    self.has_displacements = False
    for variable in self.exodus_result.current_variables:
      if 'ObjectId' not in variable:
        self.variable_contour.addItem(variable)
        if 'disp' in variable:
          self.has_displacements = True

    if self.has_displacements:
      self.displace_groupbox.setDisabled(False)

    self.block_view_model.clear()
    for block in self.exodus_result.blocks:
      block_display_name = str(block)
      if block in self.exodus_result.block_to_name:
        block_display_name += ' : ' + self.exodus_result.block_to_name[block]
        
      item = QtGui.QStandardItem(str(block_display_name))
      item.exodus_block = block
      item.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
      item.setCheckState(QtCore.Qt.Checked)
      self.block_view_model.appendRow(item)

    # Try to restore back to the view of the variable we were looking at
    found_index = self.variable_contour.findText(self.old_contour)
    if found_index != -1:
      self.variable_contour.setCurrentIndex(found_index)
    else: # If this variable doesn't exist then we are probably running a new simulation... try to reset the camera
      self._resetView()      
      
    self.time_slider.setMinimum(0)
    self.time_slider.setMaximum(self.current_max_timestep)
    
  def _blockViewItemChanged(self, item):
    if item.checkState() == QtCore.Qt.Checked:
      self.exodus_result.showBlock(item.exodus_block)
      self.exodus_result.reader.Update()
      self.exodus_result.geom.Update()
      self.current_bounds = self.exodus_result.actor.GetBounds()
      self._updateContours()
    else:
      self.exodus_result.hideBlock(item.exodus_block)
      self.exodus_result.reader.Update()
      self.exodus_result.geom.Update()
      self.current_bounds = self.exodus_result.actor.GetBounds()
      self._updateContours()

  def _displaceToggled(self, value):
    self._timeSliderReleased()

  def _displaceMagnitudeTextReturn(self):
    self.current_displacement_magnitude = float(self.displace_magnitude_text.text())
    self._timeSliderReleased()    
    
  def _drawEdgesChanged(self, value):
    if value == QtCore.Qt.Checked:
      self.exodus_result.actor.GetProperty().EdgeVisibilityOn()
      self.exodus_result.clip_actor.GetProperty().EdgeVisibilityOn()
    else:
      self.exodus_result.actor.GetProperty().EdgeVisibilityOff()
      self.exodus_result.clip_actor.GetProperty().EdgeVisibilityOff()
    self.vtkwidget.updateGL()

  def _fillComponentCombo(self, variable_name, components):
    self.variable_component.clear()
    self.variable_component.addItem('Magnitude')
    num_components = components[variable_name]
    if num_components > 1 and self.exodus_result.current_dim >= 2:
      self.variable_component.setDisabled(False)
      self.variable_component.addItem('X')
      self.variable_component.addItem('Y')
    else:
      self.variable_component.setDisabled(True)
      
    if num_components > 1 and  self.exodus_result.current_dim == 3:
      self.variable_component.addItem('Z')

    self._variableComponentSelected('Magnitude')
    
  def _contourVariableSelected(self, value):
    value_string = str(value)
    self.current_variable = value_string

    # Maybe results haven't been written yet...
    if not self.exodus_result.data.GetPointData().GetVectors(value_string) and not self.exodus_result.data.GetCellData().GetVectors(value_string):
      return

    if value_string in self.exodus_result.current_nodal_components:
      self._fillComponentCombo(value_string, self.exodus_result.current_nodal_components)
    elif value_string in self.exodus_result.current_elemental_components:
      self._fillComponentCombo(value_string, self.exodus_result.current_elemental_components)
    
  def _variableComponentSelected(self, value):
    value_string = str(value)
    if value_string == 'Magnitude':
      self.exodus_result.lut.SetVectorModeToMagnitude()
      self.component_index = -1
    elif value_string == 'X':
      self.exodus_result.lut.SetVectorModeToComponent()
      self.exodus_result.lut.SetVectorComponent(0)
      self.component_index = 0
    elif value_string == 'Y':
      self.exodus_result.lut.SetVectorModeToComponent()
      self.exodus_result.lut.SetVectorComponent(1)
      self.component_index = 1
    elif value_string == 'Z':
      self.exodus_result.lut.SetVectorModeToComponent()
      self.exodus_result.lut.SetVectorComponent(2)
      self.component_index = 2

    self._updateContours()

  def _updateContours(self):
    if self.clip_groupbox.isChecked():
      self.exodus_result.clipper.Modified()
      self.exodus_result.clipper.Update()
      self.exodus_result.clip_geom.Update()
      self.exodus_result.clip_mapper.Update()

    if self.current_variable in self.exodus_result.current_nodal_components:
      data = self.exodus_result.data.GetPointData().GetVectors(self.current_variable)
      if data:
        self.exodus_result.mapper.SetScalarModeToUsePointFieldData()
        self.exodus_result.mapper.SetScalarRange(data.GetRange(self.component_index))
        self.exodus_result.clip_mapper.SetScalarModeToUsePointFieldData()
        self.exodus_result.clip_mapper.SetScalarRange(data.GetRange(self.component_index))
    elif self.current_variable in self.exodus_result.current_elemental_components:
      data = self.exodus_result.data.GetCellData().GetVectors(self.current_variable)
      if data:
        self.exodus_result.mapper.SetScalarModeToUseCellFieldData()
        self.exodus_result.mapper.SetScalarRange(data.GetRange(self.component_index))
        self.exodus_result.clip_mapper.SetScalarModeToUseCellFieldData()
        self.exodus_result.clip_mapper.SetScalarRange(data.GetRange(self.component_index))

    self.exodus_result.mapper.SelectColorArray(self.current_variable)
    self.exodus_result.clip_mapper.SelectColorArray(self.current_variable)
    
    self.exodus_result.scalar_bar.SetTitle(self.current_variable)
    self.renderer.AddActor2D(self.exodus_result.scalar_bar)
    self.vtkwidget.updateGL()

    
  def _resetView(self):
    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()
    
  def _automaticUpdateChanged(self, value):
    if value == QtCore.Qt.Checked:
      self.automatically_update = True
    else:
      self.automatically_update = False

  def _beginningClicked(self):
    self.time_slider.setSliderPosition(0)
    self._timeSliderReleased()

  def _backClicked(self):
    self.time_slider.setSliderPosition(self.time_slider.sliderPosition()-1)
    self._timeSliderReleased()
        
  def _playClicked(self):
    self.play_button.setDisabled(True)
    self.pause_button.setDisabled(False)
    self.currently_playing = True

    first = True
    while((first or self.currently_looping) and self.currently_playing):
      first = False

      # If the slider is at the end then start over
      self.qt_app.processEvents()
      time.sleep(0.02)
      self.qt_app.processEvents()
      if self.time_slider.sliderPosition() == self.time_slider.maximum():
        self.time_slider.setSliderPosition(0)
        
      while self.time_slider.sliderPosition() < self.time_slider.maximum():
        self.time_slider.setSliderPosition(self.time_slider.sliderPosition()+1)
        self.qt_app.processEvents()
        self._timeSliderReleased()
        time.sleep(0.02)
        self.qt_app.processEvents()
        if not self.currently_playing:
          break

    self.play_button.setDisabled(False)
    self.pause_button.setDisabled(True)
    
  def _pauseClicked(self):
    self.play_button.setDisabled(False)
    self.pause_button.setDisabled(True)
    self.currently_playing = False

  def _forwardClicked(self):
    self.time_slider.setSliderPosition(self.time_slider.sliderPosition()+1)
    self._timeSliderReleased()

  def _lastClicked(self):
    self.time_slider.setSliderPosition(self.time_slider.maximum())
    self._timeSliderReleased()

  def _loopClicked(self, state):
    if state:
      self.currently_looping = True
    else:
      self.currently_looping = False
    
  def _timeSliderChanged(self):
    self.time_slider_textbox.setText(str(self.time_slider.sliderPosition()))

  def _timeSliderReleased(self):
    textbox_string = self.time_slider_textbox.text()
    if textbox_string == '':
      textbox_string = str(self.exodus_result.min_timestep)

    if int(textbox_string) in self.timestep_to_exodus_result:
      for actor in self.exodus_result.current_actors:
        self.renderer.RemoveActor(actor)
      self.exodus_result = self.timestep_to_exodus_result[int(textbox_string)]
      
      if self.clip_groupbox.isChecked():
        self.renderer.AddActor(self.exodus_result.clip_actor)
        if self.draw_edges_checkbox.checkState() == QtCore.Qt.Checked:
          self.exodus_result.clip_actor.GetProperty().EdgeVisibilityOn()
        else:
          self.exodus_result.clip_actor.GetProperty().EdgeVisibilityOff()
      else:
        self.renderer.AddActor(self.exodus_result.actor)
        if self.draw_edges_checkbox.checkState() == QtCore.Qt.Checked:
          self.exodus_result.actor.GetProperty().EdgeVisibilityOn()
        else:
          self.exodus_result.actor.GetProperty().EdgeVisibilityOff()

      num_block_view_items = self.block_view_model.rowCount()
      for i in xrange(num_block_view_items):
        item = self.block_view_model.item(i)
        if item.checkState() == QtCore.Qt.Checked:
          self.exodus_result.showBlock(item.exodus_block)
        else:
          self.exodus_result.hideBlock(item.exodus_block)

      if self.has_displacements and self.displace_groupbox.isChecked():
        self.exodus_result.reader.SetApplyDisplacements(1)
        self.exodus_result.reader.SetDisplacementMagnitude(float(self.current_displacement_magnitude))
      else:
        self.exodus_result.reader.SetApplyDisplacements(0)

    if self.exodus_result.reader:
      self.exodus_result.reader.SetTimeStep(self.timestep_to_timestep[int(textbox_string)])
      self.exodus_result.reader.Update()
      self.exodus_result.geom.Update()
      self.current_bounds = self.exodus_result.actor.GetBounds()
      self._updateContours()
      
  def _sliderTextboxReturn(self):
    self.time_slider.setSliderPosition(int(self.time_slider_textbox.text()))
    self._timeSliderReleased()

  def _associateResultsWithTimesteps(self):
    self.timestep_to_exodus_result = {}
    self.timestep_to_timestep = {}
    self.current_max_timestep = -1
    for result in self.exodus_results:
      result.reader.UpdateTimeInformation()
      min = result.reader.GetTimeStepRange()[0]
      max = result.reader.GetTimeStepRange()[1]
      for timestep in xrange(min, max+1):
        self.current_max_timestep += 1
        self.timestep_to_exodus_result[self.current_max_timestep] = result
        self.timestep_to_timestep[self.current_max_timestep] = timestep
    
  def _timestepBegin(self):
    # Check to see if there are new exodus files with adapted timesteps in them.
    if self.file_name:
      base_stamp = os.path.getmtime(self.file_name)
      for file_name in sorted(glob.glob(self.file_name + '-s*')):
        file_stamp = os.path.getmtime(file_name)
        if file_stamp >= base_stamp and file_name not in self.file_names:
          self.file_names.append(file_name)
          exodus_result = ExodusResult(self, self.plane)
          exodus_result.setFileName(file_name)
          self.exodus_results.append(exodus_result)
      
    if not self.exodus_result:
      output_file_names = self.input_file_widget.getOutputFileNames()

      output_file = ''

      for file_name in output_file_names:
        if '.e' in file_name and os.path.exists(file_name):
          self.file_name = file_name
          self.exodus_result = ExodusResult(self, self.plane)
          self.exodus_result.setFileName(file_name)
          self.exodus_results.append(self.exodus_result)
          self.current_max_timestep = self.exodus_result.max_timestep
          self.renderer.AddActor(self.exodus_result.actor)
          self._drawEdgesChanged(self.draw_edges_checkbox.checkState())

          if self.first:
            self.first = False
            self.renderer.ResetCamera()

          # Avoid z-buffer fighting
          vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

          if self.clip_groupbox.isChecked():
            _clippingToggled(True)
      
          self.vtkwidget.updateGL()
          self._updateControls()


    if self.exodus_result and self.automatically_update:
      self._associateResultsWithTimesteps()
#      self.exodus_result.reader.UpdateTimeInformation()
#      range = self.exodus_result.reader.GetTimeStepRange()
#      self.exodus_result.min_timestep = range[0]
#      self.exodus_result.max_timestep = range[1]
      self.time_slider.setMinimum(0)

      # Only automatically move forward if they're on the current step
      if self.time_slider.sliderPosition() == self.time_slider.maximum():
        self.time_slider.setMaximum(self.current_max_timestep)
        self.time_slider.setSliderPosition(self.current_max_timestep)
        self._timeSliderReleased()
        if self.clip_groupbox.isChecked():
          self._clipSliderReleased()
        self.vtkwidget.updateGL()
      else:
        self.time_slider.setMaximum(self.current_max_timestep)

  def _timestepEnd(self):
    pass
    
  def _runStarted(self):
    self.file_name = None
    self.file_names = []

    if not self.exodus_result:
      return

    for actor in self.exodus_result.current_actors:
      self.renderer.RemoveActor(actor)

    del self.exodus_result.current_actors[:]

    self.exodus_result = None
    self.exodus_results = []
    self.timestep_to_exodus_result = {}
    
  def _runStopped(self):
    self._timestepBegin()
    

  def _clippingToggled(self, value):
    if value:
      self.renderer.RemoveActor(self.exodus_result.current_actor)
      self.renderer.AddActor(self.exodus_result.clip_actor)
      self.exodus_result.current_actor = self.exodus_result.clip_actor
      self.clip_plane_slider.setSliderPosition(50)
      self._clipSliderMoved(50)
      self._clipSliderReleased()
    else:
      self.renderer.RemoveActor(self.exodus_result.current_actor)
      self.renderer.AddActor(self.exodus_result.actor)
      self.exodus_result.current_actor = self.exodus_result.actor
      
    self.vtkwidget.updateGL()    
    
  def _clipNormalChanged(self, value):
    self.plane.SetOrigin(self.current_bounds[0],
                         self.current_bounds[2],
                         self.current_bounds[4])
    if value == 'x':
      self.plane.SetNormal(1, 0, 0)
    elif value == 'y':
      self.plane.SetNormal(0, 1, 0)
    else:
      self.plane.SetNormal(0, 0, 1)

    self.clip_plane_slider.setSliderPosition(50)
    self._clipSliderMoved(50)

    self.vtkwidget.updateGL()

  def _clipSliderReleased(self):
    self._updateContours()
    self.vtkwidget.updateGL()    
  
  def _clipSliderMoved(self, value):
    direction = str(self.clip_plane_combobox.currentText())

    min = 0
    max = 0

    if direction == 'x':
      min = self.current_bounds[0]
      max = self.current_bounds[1]
    elif direction == 'y':
      min = self.current_bounds[2]
      max = self.current_bounds[3]
    elif direction == 'z':
      min = self.current_bounds[4]
      max = self.current_bounds[5]
    
    step_size = (max - min)/100.0
    steps = value
    distance = float(steps)*step_size
    position = min + distance

    old = self.plane.GetOrigin()
    self.plane.SetOrigin(position if direction == 'x' else old[0],
                         position if direction == 'y' else old[1],
                         position if direction == 'z' else old[2])    

    self._updateContours()
    self.vtkwidget.updateGL()    
