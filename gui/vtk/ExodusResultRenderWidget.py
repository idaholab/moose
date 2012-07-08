import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
import time

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
    self.plane.SetOrigin(0, 0, 0)
    self.plane.SetNormal(1, 0, 0)

    self.reader = None
    
    self.execution_widget = execution_widget
    self.execution_widget.run_started.connect(self._runStarted)
    self.execution_widget.run_stopped.connect(self._runStopped)
    self.execution_widget.timestep_begin.connect(self._timestepBegin)
    self.execution_widget.timestep_end.connect(self._timestepEnd)
    
    self.main_layout = QtGui.QVBoxLayout()
    self.setMinimumWidth(700)
    self.setLayout(self.main_layout)

    self.vtkwidget = vtk.QVTKWidget2()

    self.renderer = vtk.vtkRenderer()
    self.renderer.SetBackground(0,0,0)
    self.renderer.SetBackground2(1,1,1)
    self.renderer.SetGradientBackground(1)
    self.renderer.ResetCamera()
    
    self.main_layout.addWidget(self.vtkwidget)
    self.vtkwidget.show()

    self.current_actors = []

    self.vtkwidget.GetRenderWindow().AddRenderer(self.renderer)

    self.first = True

    self.setupControls()

  def setupControls(self):
    self.controls_widget = QtGui.QWidget()
    self.controls_layout = QtGui.QHBoxLayout()
    self.main_layout.addLayout(self.controls_layout)

    self.left_controls_layout = QtGui.QVBoxLayout()
    self.right_controls_layout = QtGui.QVBoxLayout()

    self.controls_layout.addLayout(self.left_controls_layout)
    self.controls_layout.addLayout(self.right_controls_layout)
    
    self.draw_edges_checkbox = QtGui.QCheckBox("View Mesh")
    self.draw_edges_checkbox.stateChanged[int].connect(self._drawEdgesChanged)
    self.left_controls_layout.addWidget(self.draw_edges_checkbox)

    self.automatic_update_checkbox = QtGui.QCheckBox("Automatically Update")
    self.automatic_update_checkbox.setCheckState(QtCore.Qt.Checked)
    self.automatically_update = True
    self.automatic_update_checkbox.stateChanged[int].connect(self._automaticUpdateChanged)
    self.left_controls_layout.addWidget(self.automatic_update_checkbox)

    
    self.contour_layout = QtGui.QHBoxLayout()
    self.contour_label = QtGui.QLabel("Contour:")
    self.variable_contour = QtGui.QComboBox()
    self.variable_contour.currentIndexChanged[str].connect(self._contourVariableSelected)
    self.contour_layout.addWidget(self.contour_label, alignment=QtCore.Qt.AlignRight)
    self.contour_layout.addWidget(self.variable_contour, alignment=QtCore.Qt.AlignLeft)
    self.left_controls_layout.addLayout(self.contour_layout)

    self.beginning_button = QtGui.QToolButton()
    self.beginning_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrFirst32.png'))
    self.beginning_button.clicked.connect(self._beginningClicked)

    self.back_button = QtGui.QToolButton()
    self.back_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrBack32.png'))
    self.back_button.clicked.connect(self._backClicked)

    self.play_button = QtGui.QToolButton()
    self.play_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrPlay32.png'))
    self.play_button.clicked.connect(self._playClicked)

    self.pause_button = QtGui.QToolButton()
    self.pause_button.setDisabled(True)
    self.pause_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrPause32.png'))
    self.pause_button.clicked.connect(self._pauseClicked)

    self.forward_button = QtGui.QToolButton()
    self.forward_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrForward32.png'))
    self.forward_button.clicked.connect(self._forwardClicked)

    self.last_button = QtGui.QToolButton()
    self.last_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrLast32.png'))
    self.last_button.clicked.connect(self._lastClicked)

    self.loop_button = QtGui.QToolButton()
    self.loop_button.setCheckable(True)
    self.loop_button.setIcon(QtGui.QIcon(pathname + '/resources/from_paraview/pqVcrLoop24.png'))
    self.loop_button.toggled.connect(self._loopClicked)

    self.currently_looping = False

    self.time_slider_label = QtGui.QLabel("Timestep:")
    self.time_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
    self.time_slider.setMaximumWidth(600)
    
    self.time_slider.valueChanged.connect(self._timeSliderChanged)
    self.time_slider.sliderReleased.connect(self._timeSliderReleased)
    
    self.time_slider_textbox = QtGui.QLineEdit()
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

    self.right_controls_layout.addWidget(self.clip_groupbox)

  def _updateControls(self):
    self.variable_contour.clear()
    for variable in self.current_variables:
      self.variable_contour.addItem(variable)
    self.time_slider.setMinimum(self.min_timestep)
    self.time_slider.setMaximum(self.max_timestep)
    self.time_slider.setSliderPosition(self.max_timestep)
    
  def setFileName(self, file_name):      
    self.currently_has_actor = True
    
    self.file_name = file_name
    self.reader = vtk.vtkExodusIIReader()
    self.reader.SetFileName(self.file_name)
    self.reader.UpdateInformation()

    self.min_timestep = 0
    self.max_timestep = 0
    range = self.reader.GetTimeStepRange()
    self.min_timestep = range[0]
    self.max_timestep = range[1]

    self.reader.SetAllArrayStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1)
    self.reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
    self.reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL_TEMPORAL, 1)
    self.reader.SetTimeStep(self.max_timestep)
    self.reader.Update()
    self.current_variable_point_data = {}
    self.current_variables = []

    num_variables = self.reader.GetNumberOfObjectArrays(vtk.vtkExodusIIReader.NODAL)
    for var_num in xrange(num_variables):
      self.current_variables.append(self.reader.GetObjectArrayName(vtk.vtkExodusIIReader.NODAL,var_num))

    cdp = vtk.vtkCompositeDataPipeline()
    vtk.vtkAlgorithm.SetDefaultExecutivePrototype(cdp)

    self.output = self.reader.GetOutput()
#    self.geom = vtk.vtkCompositeDataGeometryFilter()
#    self.geom.SetInputConnection(0,self.reader.GetOutputPort(0));
#    self.geom.Update()

    lut = vtk.vtkLookupTable()
    lut.SetHueRange(0.667, 0.0)
    lut.SetNumberOfColors(256)
    lut.Build()

#    self.data = self.geom.GetOutput()

    
#    self.mapper = vtk.vtkPolyDataMapper()
#    self.mapper.SetInput(self.data)
#    self.mapper.ScalarVisibilityOn()
#    self.mapper.SetColorModeToMapScalars()
#    self.mapper.SetLookupTable(lut)


    self.mesh = self.output.GetBlock(0).GetBlock(0)
    
    self.geom = vtk.vtkGeometryFilter()
    self.geom.SetInput(self.mesh)
    self.geom.Update()
    
    self.mapper = vtk.vtkDataSetMapper()
    self.mapper.SetInput(self.mesh)

    
    self.actor = vtk.vtkActor()
    self.current_actors.append(self.actor)
    self.actor.SetMapper(self.mapper)
    self.renderer.AddActor(self.actor)
    self.current_actor = self.actor

#     self.clipper = vtk.vtkClipPolyData()
#     self.clipper.SetInput(self.data)
#     self.clipper.SetClipFunction(self.plane)
#     self.clipper.GenerateClipScalarsOff()

#     self.clip_mapper = vtk.vtkPolyDataMapper()
#     self.clip_mapper.SetInputConnection(self.clipper.GetOutputPort())
#     self.clip_mapper.ScalarVisibilityOn()
#     self.clip_mapper.SetColorModeToMapScalars()
#     self.clip_mapper.SetLookupTable(lut)

#     self.clip_actor = vtk.vtkActor()
#     self.clip_actor.SetMapper(self.clip_mapper)

    self.actor.GetProperty().SetRepresentationToWireframe()





#     self.cutter = vtk.vtkCutter()
#     self.cutter.SetInput(self.data)
#     self.cutter.SetCutFunction(self.plane)
#     self.cutter.GenerateCutScalarsOff()
    
#     self.cut_mapper = vtk.vtkPolyDataMapper()
#     self.cut_mapper.SetInputConnection(self.cutter.GetOutputPort())
# #    self.cut_mapper.SetScalarRange(pl3d.GetOutput().GetPointData().GetScalars().GetRange())
#     self.cut_actor = vtk.vtkActor()
#     self.cut_actor.SetMapper(self.cut_mapper)

#    extract = vtk.vtkExtractGeometry()
#    extract.SetInput(self.data)
#    extract.SetImplicitFunction(self.plane)
#    dataMapper = vtk.vtkDataSetMapper()
#    dataMapper.SetInputConnection(extract.GetOutputPort())
#    self.cut_actor = vtk.vtkActor()
#    self.cut_actor.SetMapper(dataMapper)









    

    self._drawEdgesChanged(self.draw_edges_checkbox.checkState())

    self.scalar_bar = vtk.vtkScalarBarActor()
    self.current_actors.append(self.scalar_bar)
    self.scalar_bar.SetLookupTable(self.mapper.GetLookupTable())
    self.scalar_bar.SetNumberOfLabels(4)
    
    self.current_bounds = self.actor.GetBounds()

    # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

    if self.first:
      self.first = False
      self.renderer.ResetCamera()
      
    self.vtkwidget.updateGL()

    self._updateControls()

  def _drawEdgesChanged(self, value):
    if value == QtCore.Qt.Checked:
      self.actor.GetProperty().EdgeVisibilityOn()
    else:
      self.actor.GetProperty().EdgeVisibilityOff()
    self.vtkwidget.updateGL()

  def _contourVariableSelected(self, value, force_update=False):
    value_string = str(value)

    if force_update:
      del self.current_variable_point_data[value_string]

    if value_string not in self.current_variable_point_data:
      self.current_variable_point_data[value_string] = self.data.GetPointData().GetArray(value_string)

    point_data = self.current_variable_point_data[value_string]
    self.data.GetPointData().SetScalars(point_data)
    self.mapper.SetScalarRange(point_data.GetRange()[0],point_data.GetRange()[1])
    self.clip_mapper.SetScalarRange(point_data.GetRange()[0],point_data.GetRange()[1])
    self.scalar_bar.SetTitle(value_string)
    self.renderer.AddActor2D(self.scalar_bar)
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
      textbox_string = str(self.min_timestep)

    if self.reader:
      self.reader.SetTimeStep(int(textbox_string))
      self.reader.Update()
      self.geom.Update()
      self._contourVariableSelected(self.variable_contour.currentText(), True)
      self.current_bounds = self.actor.GetBounds()

  def _sliderTextboxReturn(self):
    self.time_slider.setSliderPosition(int(self.time_slider_textbox.text()))
    self._timeSliderReleased()

  def _timestepBegin(self):
    if not self.file_name:
      output_file_names = self.input_file_widget.getOutputFileNames()

      output_file = ''

      for file_name in output_file_names:
        if '.e' in file_name and os.path.exists(file_name):
          self.setFileName(file_name)

    if self.automatically_update:
      self.reader.UpdateTimeInformation()
      range = self.reader.GetTimeStepRange()
      self.min_timestep = range[0]
      self.max_timestep = range[1]
      self.time_slider.setMinimum(self.min_timestep)

      # Only automatically move forward if they're on the current step
      if self.time_slider.sliderPosition() == self.time_slider.maximum():
        self.time_slider.setMaximum(self.max_timestep)
        self.time_slider.setSliderPosition(self.max_timestep)
        self._timeSliderReleased()
      else:
        self.time_slider.setMaximum(self.max_timestep)

  def _timestepEnd(self):
    pass
    
  def _runStarted(self):
    self.file_name = None
    
    for actor in self.current_actors:
      self.renderer.RemoveActor(actor)

    del self.current_actors[:]
    
  def _runStopped(self):
    self._timestepBegin()
    

  def _clippingToggled(self, value):
    if value:
      self.renderer.RemoveActor(self.current_actor)
#      self.renderer.AddActor(self.clip_actor)
#      self.current_actor = self.clip_actor

      self.renderer.AddActor(self.cut_actor)
      self.current_actor = self.cut_actor

    else:
      self.renderer.RemoveActor(self.current_actor)
      self.renderer.AddActor(self.actor)
      self.current_actor = self.actor
      
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

    self.vtkwidget.updateGL()    
