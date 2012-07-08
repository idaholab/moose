import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class ExodusResultRenderWidget(QtGui.QWidget):
  def __init__(self, input_file_widget, execution_widget):
    QtGui.QWidget.__init__(self)
    self.input_file_widget = input_file_widget
    
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
#    self.controls_widget.setLayout(self.controls_layout)
#    self.controls_widget.setMinimumHeight(100)
#    self.main_layout.addWidget(self.controls_widget)
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
    self.contour_label = QtGui.QLabel("Variable Contour:")
    self.variable_contour = QtGui.QComboBox()
    self.variable_contour.currentIndexChanged[str].connect(self._contourVariableSelected)
    self.contour_layout.addWidget(self.contour_label, alignment=QtCore.Qt.AlignRight)
    self.contour_layout.addWidget(self.variable_contour, alignment=QtCore.Qt.AlignLeft)
    self.right_controls_layout.addLayout(self.contour_layout)
    
    self.time_slider_label = QtGui.QLabel("Timestep:")
    self.time_slider = QtGui.QSlider(QtCore.Qt.Horizontal)
    self.time_slider.setMaximumWidth(600)
    
    self.time_slider.valueChanged.connect(self._timeSliderChanged)
    self.time_slider.sliderReleased.connect(self._timeSliderReleased)
    
    self.time_slider_textbox = QtGui.QLineEdit()
    self.time_slider_textbox.setMaximumWidth(30)
    self.time_slider_textbox.returnPressed.connect(self._sliderTextboxReturn)
    
    self.slider_layout = QtGui.QHBoxLayout()
    self.slider_layout.addWidget(self.time_slider_label, alignment=QtCore.Qt.AlignRight)
    self.slider_layout.addWidget(self.time_slider)
    self.slider_layout.addWidget(self.time_slider_textbox, alignment=QtCore.Qt.AlignLeft)
    self.right_controls_layout.addLayout(self.slider_layout)

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

    self.geom = vtk.vtkCompositeDataGeometryFilter()
    self.geom.SetInputConnection(0,self.reader.GetOutputPort(0));
    self.geom.Update()

    lut = vtk.vtkLookupTable()
    lut.SetHueRange(0.667, 0.0)
    lut.SetNumberOfColors(256)
    lut.Build()

    self.data = self.geom.GetOutput()
    self.mapper = vtk.vtkPolyDataMapper()
    self.mapper.SetInput(self.data)
    self.mapper.ScalarVisibilityOn()
    self.mapper.SetColorModeToMapScalars()
    self.mapper.SetLookupTable(lut)
    
    self.actor = vtk.vtkActor()
    self.current_actors.append(self.actor)
    self.actor.SetMapper(self.mapper)
    self.renderer.AddActor(self.actor)

    self.edge_geom = vtk.vtkCompositeDataGeometryFilter()
    self.edge_geom.SetInputConnection(0,self.reader.GetOutputPort(0));
    self.edge_geom.Update()

    self.edges = vtk.vtkExtractEdges()
    self.edges.SetInput(self.edge_geom.GetOutput())
    self.edge_mapper = vtk.vtkPolyDataMapper()
    self.edge_mapper.SetInput(self.edges.GetOutput())

    self.edge_actor = vtk.vtkActor()
    self.current_actors.append(self.edge_actor)
    self.edge_actor.SetMapper(self.edge_mapper)
    self.edge_actor.GetProperty().SetColor(0,0,0)

    self._drawEdgesChanged(self.draw_edges_checkbox.checkState())

    self.scalar_bar = vtk.vtkScalarBarActor()
    self.current_actors.append(self.scalar_bar)
    self.scalar_bar.SetLookupTable(self.mapper.GetLookupTable())
    self.scalar_bar.SetNumberOfLabels(4)

    # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

    if self.first:
      self.first = False
      self.renderer.ResetCamera()
      
    self.vtkwidget.updateGL()

    self._updateControls()

  def _drawEdgesChanged(self, value):
    if value == QtCore.Qt.Checked:
      self.renderer.AddActor(self.edge_actor)
    else:
      self.renderer.RemoveActor(self.edge_actor)
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
    self.scalar_bar.SetTitle(value_string)
    self.renderer.AddActor2D(self.scalar_bar)
    self.vtkwidget.updateGL()

  def _automaticUpdateChanged(self, value):
    if value == QtCore.Qt.Checked:
      self.automatically_update = True
    else:
      self.automatically_update = False
    
  def _timeSliderChanged(self):
    self.time_slider_textbox.setText(str(self.time_slider.sliderPosition()))

  def _timeSliderReleased(self):
    textbox_string = self.time_slider_textbox.text()
    if textbox_string == '':
      textbox_string = str(self.min_timestep)
      
    self.reader.SetTimeStep(int(textbox_string))
    self.reader.Update()
    self.geom.Update()
    self.edge_geom.Update()
    self._contourVariableSelected(self.variable_contour.currentText(), True)

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
      self.time_slider.setMaximum(self.max_timestep)
      self.time_slider.setSliderPosition(self.max_timestep)
      self._timeSliderReleased()

  def _timestepEnd(self):
    pass
    
  def _runStarted(self):
    self.file_name = None
    
    for actor in self.current_actors:
      self.renderer.RemoveActor(actor)

    del self.current_actors[:]
    
  def _runStopped(self):
    self._timestepBegin()
    

