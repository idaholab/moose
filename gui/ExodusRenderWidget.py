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
    self.this_layout = QtGui.QHBoxLayout()
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
      
    self.currently_has_actor = True
    
    self.file_name = file_name
    reader = vtk.vtkExodusIIReader()
    reader.SetFileName(self.file_name)
    reader.UpdateInformation()
    reader.Update()

    out = reader.GetOutput()
    vtk_mesh = []
    for i in xrange( out.GetNumberOfBlocks() ):
        blk = out.GetBlock( i )
        for j in xrange( blk.GetNumberOfBlocks() ):
            sub_block = blk.GetBlock( j )
            if sub_block and sub_block.IsA( 'vtkUnstructuredGrid' ):
                vtk_mesh.append( sub_block )    

    for mesh in vtk_mesh:
      grid_mapper = vtk.vtkDataSetMapper()
      grid_mapper.SetInput(mesh)
      grid_actor = vtk.vtkActor()
      grid_actor.SetMapper(grid_mapper)

      self.current_actors.append(grid_actor)
      self.renderer.AddActor(grid_actor)

      edges = vtk.vtkExtractEdges()
      edges.SetInput(mesh)
      edge_mapper = vtk.vtkPolyDataMapper()
      edge_mapper.SetInput(edges.GetOutput())

      edge_actor = vtk.vtkActor()
      edge_actor.SetMapper(edge_mapper)
      edge_actor.GetProperty().SetColor(0,0,0)

      self.current_actors.append(edge_actor)
      self.renderer.AddActor(edge_actor)

    # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()


    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()
