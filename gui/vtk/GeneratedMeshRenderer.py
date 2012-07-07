import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
from vtk.util.colors import peacock, tomato, red, white, black

from GeneratedMeshActor import GeneratedMeshActor
from ClippedActor import ClippedActor

from MeshRenderer import MeshRenderer

class GeneratedMeshRenderer(MeshRenderer):
  def __init__(self, render_widget, mesh_item_data):
    MeshRenderer.__init__(self, render_widget, mesh_item_data)

    self.dim = int(mesh_item_data['dim'])

    self.xmin = 0.0
    self.ymin = 0.0
    self.zmin = 0.0

    self.xmax = 1.0 
    self.ymax = 1.0
    self.zmax = 1.0 

    if 'xmin' in mesh_item_data:
      self.xmin = float(mesh_item_data['xmin'])
    if 'ymin' in mesh_item_data:
      self.ymin = float(mesh_item_data['ymin'])
    if 'zmin' in mesh_item_data:
      self.zmin = float(mesh_item_data['zmin'])

    if 'xmax' in mesh_item_data:
      self.xmax = float(mesh_item_data['xmax'])
    if 'ymax' in mesh_item_data:
      self.ymax = float(mesh_item_data['ymax'])
    if 'zmax' in mesh_item_data:
      self.zmax = float(mesh_item_data['zmax'])

    # This is actually going to be the number of nodes in each direction
    # The default is one element in each direction... meaning 2 nodes
    # But if we are in a lower dimension then there is just one node...
    self.nx = 2
    self.ny = 1
    self.nz = 1

    if self.dim >= 2:
      self.ny = 2 

    if self.dim == 3:
      self.nz = 2

    # The plus 1's in here is because we're reading number of _elements_ but we need number of nodes
    if 'nx' in mesh_item_data:
      self.nx = int(mesh_item_data['nx'])+1
    if self.dim >= 2 and 'ny' in mesh_item_data:
      self.ny = int(mesh_item_data['ny'])+1
    if self.dim ==3 and 'nz' in mesh_item_data:
      self.nz = int(mesh_item_data['nz'])+1

    self.dx = 0.0
    self.dy = 0.0
    self.dz = 0.0
    
    if self.nx:
      self.dx = (self.xmax - self.xmin)/float(self.nx)

    if self.ny:
      self.dy = (self.ymax - self.ymin)/float(self.ny)

    if self.nz:
      self.dz = (self.zmax - self.zmin)/float(self.nz)
    
    self.x_coords = vtk.vtkFloatArray()
    self.y_coords = vtk.vtkFloatArray()
    self.z_coords = vtk.vtkFloatArray()

    for i in xrange(self.nx):
      self.x_coords.InsertNextValue(float(i)*self.dx)

    for j in xrange(self.ny):
      self.y_coords.InsertNextValue(float(j)*self.dy)

    for k in xrange(self.nz):
      self.z_coords.InsertNextValue(float(k)*self.dz)

    self.main_block = self.generateMesh(0,0,0,True,True,True)
    
    self.block_actors['0'] = GeneratedMeshActor(self.renderer, self.main_block)

  def generateMesh(self, x, y, z, in_x, in_y, in_z):
    x_coord = vtk.vtkFloatArray()
    x_coord.InsertNextValue(x)

    y_coord = vtk.vtkFloatArray()
    y_coord.InsertNextValue(y)

    z_coord = vtk.vtkFloatArray()
    z_coord.InsertNextValue(z)

    grid = vtk.vtkRectilinearGrid()
    grid.SetDimensions(self.nx if in_x else 1, self.ny if in_y else 1, self.nz if in_z else 1)
    grid.SetXCoordinates(self.x_coords if in_x else x_coord);
    grid.SetYCoordinates(self.y_coords if in_y else y_coord);
    grid.SetZCoordinates(self.z_coords if in_z else z_coord);

    return grid
    
    
