import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
import time


class ExodusResult:
  def __init__(self, render_widget, renderer, plane):
    self.render_widget = render_widget
    self.renderer = renderer
    self.plane = plane

    self.current_actors = []

  def setFileName(self, file_name):      
    self.currently_has_actor = True
    
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
    self.current_variables = []
    self.current_nodal_components = {}
    self.current_elemental_components = {}
    self.component_index = -1


    cdp = vtk.vtkCompositeDataPipeline()
    vtk.vtkAlgorithm.SetDefaultExecutivePrototype(cdp)

    self.output = self.reader.GetOutput()
    self.geom = vtk.vtkCompositeDataGeometryFilter()
    self.geom.SetInputConnection(0,self.reader.GetOutputPort(0))
    self.geom.Update()

    self.lut = vtk.vtkLookupTable()
    self.lut.SetHueRange(0.667, 0.0)
    self.lut.SetNumberOfColors(256)
    self.lut.Build()

    self.data = self.geom.GetOutput()

    num_nodal_variables = self.data.GetPointData().GetNumberOfArrays()
    for var_num in xrange(num_nodal_variables):
      var_name = self.data.GetPointData().GetArrayName(var_num)
      self.current_variables.append(var_name)
      components = self.data.GetPointData().GetVectors(var_name).GetNumberOfComponents()
      self.current_nodal_components[var_name] = components
      # self.data.GetPointData().GetVectors(value_string).GetComponentName(0)

    num_elemental_variables = self.data.GetCellData().GetNumberOfArrays()
    for var_num in xrange(num_elemental_variables):
      var_name = self.data.GetCellData().GetArrayName(var_num)
      self.current_variables.append(var_name)
      components = self.data.GetCellData().GetVectors(var_name).GetNumberOfComponents()
      self.current_elemental_components[var_name] = components      
    
    self.mapper = vtk.vtkPolyDataMapper()
    self.mapper.SetInput(self.data)
    self.mapper.ScalarVisibilityOn()
    self.mapper.SetLookupTable(self.lut)
    
    self.actor = vtk.vtkActor()
    self.current_actors.append(self.actor)
    self.actor.SetMapper(self.mapper)
    self.renderer.AddActor(self.actor)
    self.current_actor = self.actor

    self.clipper = vtk.vtkTableBasedClipDataSet()
    self.clipper.SetInput(self.output)
    self.clipper.SetClipFunction(self.plane)
    self.clipper.Update()

    self.clip_geom = vtk.vtkCompositeDataGeometryFilter()
    self.clip_geom.SetInputConnection(0,self.clipper.GetOutputPort(0))
    self.clip_geom.Update()
    
    self.clip_data = self.clip_geom.GetOutput()

    self.clip_mapper = vtk.vtkPolyDataMapper()
    self.clip_mapper.SetInput(self.clip_data)
    self.clip_mapper.ScalarVisibilityOn()
    self.clip_mapper.SetLookupTable(self.lut)

    self.clip_actor = vtk.vtkActor()
    self.clip_actor.SetMapper(self.clip_mapper)
    self.current_actors.append(self.clip_actor)

    self.scalar_bar = vtk.vtkScalarBarActor()
    self.current_actors.append(self.scalar_bar)
    self.scalar_bar.SetLookupTable(self.mapper.GetLookupTable())
    self.scalar_bar.SetNumberOfLabels(4)
    
    self.current_bounds = self.actor.GetBounds()

