import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

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
    reader.UpdateInformation()
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL_TEMPORAL, 1)
    reader.SetTimeStep(1)
    reader.Update()


    cdp = vtk.vtkCompositeDataPipeline()
    vtk.vtkAlgorithm.SetDefaultExecutivePrototype(cdp)

    geom = vtk.vtkCompositeDataGeometryFilter()
    geom.SetInputConnection(0,reader.GetOutputPort(0));
    geom.Update()

    data = geom.GetOutput()
    data.GetPointData().SetScalars(data.GetPointData().GetArray("u"))
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInput(data)
    mapper.ScalarVisibilityOn()
    mapper.SetColorModeToMapScalars()
    mapper.SetScalarRange(0,1.0)

    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    self.current_actors.append(actor)
    self.renderer.AddActor(actor)

    edge_geom = vtk.vtkCompositeDataGeometryFilter()
    edge_geom.SetInputConnection(0,reader.GetOutputPort(0));
    edge_geom.Update()

    edges = vtk.vtkExtractEdges()
    edges.SetInput(edge_geom.GetOutput())
    edge_mapper = vtk.vtkPolyDataMapper()
    edge_mapper.SetInput(edges.GetOutput())

    edge_actor = vtk.vtkActor()
    self.current_actors.append(edge_actor)
    edge_actor.SetMapper(edge_mapper)
    edge_actor.GetProperty().SetColor(0,0,0)
    self.renderer.AddActor(edge_actor)

    # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()


    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()

class ExodusResultRenderWidget(ExodusRenderWidget):
  def __init__(self):
    ExodusRenderWidget.__init__(self)
    self.setFileName('out.e')
