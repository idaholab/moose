import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
import time


class ExodusResult:
  def __init__(self, render_widget, plane):
    self.render_widget = render_widget
    self.plane = plane

    self.current_actors = []

  def setFileName(self, file_name, lut):

    try:
      self.currently_has_actor = True
      self.lut = lut

      self.file_name = file_name
      self.reader = vtk.vtkExodusIIReader()
      self.reader.SetFileName(self.file_name)
      self.reader.UpdateInformation()

      self.current_dim = self.reader.GetDimensionality()

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
      self.current_nodal_variables = []
      self.current_elemental_variables = []
      self.current_nodal_components = {}
      self.current_elemental_components = {}
      self.component_index = -1

      num_blocks = self.reader.GetNumberOfElementBlockArrays()
      self.blocks = set()
      self.block_to_name = {}
      for i in xrange(num_blocks):
        block_num = self.reader.GetObjectId(vtk.vtkExodusIIReader.ELEM_BLOCK,i)
        self.blocks.add(block_num)
        if 'Unnamed' not in self.reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' '):
          self.block_to_name[block_num] = self.reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' ')[0]

      cdp = vtk.vtkCompositeDataPipeline()
      vtk.vtkAlgorithm.SetDefaultExecutivePrototype(cdp)

      self.output = self.reader.GetOutput()
      self.geom = vtk.vtkCompositeDataGeometryFilter()
      self.geom.SetInputConnection(0,self.reader.GetOutputPort(0))
      self.geom.Update()

      self.data = self.geom.GetOutput()

      num_nodal_variables = self.data.GetPointData().GetNumberOfArrays()
      for var_num in xrange(num_nodal_variables):
        var_name = self.data.GetPointData().GetArrayName(var_num)
        self.current_nodal_variables.append(var_name)
        components = self.data.GetPointData().GetVectors(var_name).GetNumberOfComponents()
        self.current_nodal_components[var_name] = components
        # self.data.GetPointData().GetVectors(value_string).GetComponentName(0)

      num_elemental_variables = self.data.GetCellData().GetNumberOfArrays()
      for var_num in xrange(num_elemental_variables):
        var_name = self.data.GetCellData().GetArrayName(var_num)
        self.current_elemental_variables.append(var_name)
        components = self.data.GetCellData().GetVectors(var_name).GetNumberOfComponents()
        self.current_elemental_components[var_name] = components

      self.application_filter = self.render_widget.application.filterResult(self.geom)

      self.mapper = vtk.vtkPolyDataMapper()
  #    self.mapper.SetInputConnection(self.tf.GetOutputPort())
      self.mapper.SetInputConnection(self.application_filter.GetOutputPort())
      self.mapper.ScalarVisibilityOn()
      self.mapper.SetLookupTable(lut)
      self.mapper.SetColorModeToMapScalars()
      self.mapper.InterpolateScalarsBeforeMappingOn()

      self.actor = vtk.vtkActor()
      self.current_actors.append(self.actor)
      self.actor.SetMapper(self.mapper)
      self.current_actor = self.actor

      self.clipper = vtk.vtkTableBasedClipDataSet()
      self.clipper.SetInput(self.output)
      self.clipper.SetClipFunction(self.plane)
      self.clipper.Update()

      self.clip_geom = vtk.vtkCompositeDataGeometryFilter()
      self.clip_geom.SetInputConnection(0,self.clipper.GetOutputPort(0))
      self.clip_geom.Update()

      self.clip_data = self.clip_geom.GetOutput()

      self.clip_application_filter = self.render_widget.application.filterResult(self.clip_geom)

      self.clip_mapper = vtk.vtkPolyDataMapper()
      self.clip_mapper.SetInputConnection(self.clip_application_filter.GetOutputPort())
      self.clip_mapper.ScalarVisibilityOn()
      self.clip_mapper.SetLookupTable(lut)

      self.clip_actor = vtk.vtkActor()
      self.clip_actor.SetMapper(self.clip_mapper)
      self.current_actors.append(self.clip_actor)

      self.scalar_bar = vtk.vtkScalarBarActor()
      self.current_actors.append(self.scalar_bar)
      self.scalar_bar.SetLookupTable(self.mapper.GetLookupTable())
      self.scalar_bar.SetNumberOfLabels(4)

      self.current_bounds = self.actor.GetBounds()
    except:
      pass

  def setColorScheme(self, lut):
    self.mapper.SetLookupTable(lut)
    self.clip_mapper.SetLookupTable(lut)
    self.scalar_bar.SetLookupTable(lut)

  def hideBlock(self, block_num):
    self.reader.SetElementBlockArrayStatus(self.reader.GetElementBlockArrayName(self.reader.GetObjectIndex(vtk.vtkExodusIIReader.ELEM_BLOCK, block_num)), 0)

  def showBlock(self, block_num):
    self.reader.SetElementBlockArrayStatus(self.reader.GetElementBlockArrayName(self.reader.GetObjectIndex(vtk.vtkExodusIIReader.ELEM_BLOCK, block_num)), 1)
