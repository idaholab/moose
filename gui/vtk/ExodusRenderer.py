import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui
import vtk
from vtk.util.colors import peacock, tomato, red, white, black

from ExodusActor import ExodusActor
from ClippedActor import ClippedActor

from MeshRenderer import MeshRenderer

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class ExodusMap:
  # These are the blocks from the multiblockdataset that correspond to each item
  element_vtk_block = 0
  sideset_vtk_block = 4
  nodeset_vtk_block = 7

class ExodusRenderer(MeshRenderer):
  def __init__(self, render_widget, mesh_item_data):
    MeshRenderer.__init__(self, render_widget, mesh_item_data)
    self.file_name = mesh_item_data['file']

    self.buildActors(self.file_name)
    
  def buildActors(self, file_name):
    reader = vtk.vtkExodusIIReader()
    reader.SetFileName(self.file_name)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.EDGE_SET, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.SIDE_SET, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODE_SET, 1)
    reader.SetAllArrayStatus(vtk.vtkExodusIIReader.NODAL_TEMPORAL, 1)
    reader.UpdateInformation()
    reader.SetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 0, 1)

    num_sidesets = reader.GetNumberOfSideSetArrays()
    num_nodesets = reader.GetNumberOfNodeSetArrays()
    num_blocks = reader.GetNumberOfElementBlockArrays()

    self.sidesets = []
    self.sideset_id_to_exodus_block = {}
    self.sideset_id_to_name = {}
    self.name_to_sideset_id = {}
    for i in xrange(num_sidesets):
      sideset_id = reader.GetObjectId(vtk.vtkExodusIIReader.SIDE_SET,i)
      self.sidesets.append(sideset_id)
      self.sideset_id_to_exodus_block[sideset_id] = i
      reader.SetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, i, 1)
      name = reader.GetObjectName(vtk.vtkExodusIIReader.SIDE_SET,i).split(' ')
      if 'Unnamed' not in name:
        self.sideset_id_to_name[sideset_id] = name[0]
        self.name_to_sideset_id[name[0]] = sideset_id

    self.nodesets = []
    self.nodeset_id_to_exodus_block = {}
    self.nodeset_id_to_name = {}
    self.name_to_nodeset_id = {}
    for i in xrange(num_nodesets):
      nodeset_id = reader.GetObjectId(vtk.vtkExodusIIReader.NODE_SET,i)
      self.nodesets.append(nodeset_id)
      self.nodeset_id_to_exodus_block[nodeset_id] = i
      reader.SetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, i, 1)
      name = reader.GetObjectName(vtk.vtkExodusIIReader.NODE_SET,i).split(' ')
      if 'Unnamed' not in name:
        self.nodeset_id_to_name[nodeset_id] = name[0]
        self.name_to_nodeset_id[name[0]] = nodeset_id

    self.blocks = []
    self.block_id_to_exodus_block = {}
    self.block_id_to_name = {}
    self.name_to_block_id = {}
    for i in xrange(num_blocks):
      block_id = reader.GetObjectId(vtk.vtkExodusIIReader.ELEM_BLOCK,i)
      self.blocks.append(block_id)
      self.block_id_to_exodus_block[block_id] = i    
      name = reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' ')
      if 'Unnamed' not in name:
        self.block_id_to_name[block_id] = name[0]
        self.name_to_block_id[name[0]] = block_id
        
    reader.SetTimeStep(1)
    reader.Update()

    self.data = reader.GetOutput()
    
    for i in xrange(num_sidesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.sideset_vtk_block, i)
      self.sideset_actors[str(self.sidesets[i])] = actor
      self.all_actors.append(actor)

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_sideset_actors[str(self.sidesets[i])] = clipped_actor
      self.all_actors.append(clipped_actor)
      
    for i in xrange(num_nodesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.nodeset_vtk_block, i)
      self.nodeset_actors[str(self.nodesets[i])] = actor
      self.all_actors.append(actor)

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_nodeset_actors[str(self.nodesets[i])] = clipped_actor
      self.all_actors.append(clipped_actor)

    for i in xrange(num_blocks):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.element_vtk_block, i)
      self.block_actors[str(self.blocks[i])] = actor
      self.all_actors.append(actor)

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_block_actors[str(self.blocks[i])] = clipped_actor
      self.all_actors.append(clipped_actor)
    
