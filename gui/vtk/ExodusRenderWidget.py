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

from PeacockActor import PeacockActor
from ExodusActor import ExodusActor
from ClippedActor import ClippedActor

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class ExodusMap:
  # These are the blocks from the multiblockdataset that correspond to each item
  element_vtk_block = 0
  sideset_vtk_block = 4
  nodeset_vtk_block = 7

class ExodusRenderer:
  def __init__(self, render_widget):
    self.render_widget = render_widget
    self.renderer = self.render_widget.renderer
    self.vtkwidget = self.renderer.vtkwidget

  def setFileName(self, file_name):
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
    for i in xrange(num_sidesets):
      sideset_id = reader.GetObjectId(vtk.vtkExodusIIReader.SIDE_SET,i)
      self.sidesets.append(sideset_id)
      self.sideset_id_to_exodus_block[sideset_id] = i
      reader.SetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, i, 1)

    self.nodesets = []
    self.nodeset_id_to_exodus_block = {}
    for i in xrange(num_nodesets):
      nodeset_id = reader.GetObjectId(vtk.vtkExodusIIReader.NODE_SET,i)
      self.nodesets.append(nodeset_id)
      self.nodeset_id_to_exodus_block[nodeset_id] = i
      reader.SetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, i, 1)

    self.blocks = []
    self.block_id_to_exodus_block = {}
    for i in xrange(num_blocks):
      block_id = reader.GetObjectId(vtk.vtkExodusIIReader.ELEM_BLOCK,i)
      self.blocks.append(block_id)
      self.block_id_to_exodus_block[block_id] = i

    reader.SetTimeStep(1)
    reader.Update()

    self.data = reader.GetOutput()

    self.all_actors = []

    self.sideset_actors = {}
    self.clipped_sideset_actors = {}

    self.current_sideset_actors = self.sideset_actors
    for i in xrange(num_sidesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.sideset_vtk_block, i)
      actor.setColor(red)
      self.sideset_actors[str(self.sidesets[i])] = actor
      self.all_actors.append(actor)

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_sideset_actors[str(self.sidesets[i])] = clipped_actor
      self.all_actors.append(clipped_actor)

      name = reader.GetObjectName(vtk.vtkExodusIIReader.SIDE_SET,i).split(' ')
      if 'Unnamed' not in name:
        self.sideset_actors[name[0]] = actor
        self.clipped_sideset_actors[name[0]] = clipped_actor

    self.nodeset_actors = {}
    self.clipped_nodeset_actors = {}

    self.current_nodeset_actors = self.nodeset_actors
    for i in xrange(num_nodesets):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.nodeset_vtk_block, i)
      actor.setColor(red)
      self.nodeset_actors[str(self.nodesets[i])] = actor
      self.all_actors.append(actor)

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_nodeset_actors[str(self.nodesets[i])] = clipped_actor
      self.all_actors.append(clipped_actor)

      name = reader.GetObjectName(vtk.vtkExodusIIReader.NODE_SET,i).split(' ')
      if 'Unnamed' not in name:
        self.nodeset_actors[name[0]] = actor
        self.clipped_nodeset_actors[name[0]] = clipped_actor

    self.block_actors = {}
    self.clipped_block_actors = {}

    self.current_block_actors = self.block_actors
    for i in xrange(num_blocks):
      actor = ExodusActor(self.renderer, self.data, ExodusMap.element_vtk_block, i)
      self.block_actors[str(self.blocks[i])] = actor
      self.all_actors.append(actor)

      actor.show()
      actor.showEdges()

      clipped_actor = ClippedActor(actor, self.plane)
      self.clipped_block_actors[str(self.blocks[i])] = clipped_actor
      self.all_actors.append(clipped_actor)

      name = reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' ')
      if 'Unnamed' not in name:
        self.block_actors[name[0]] = actor
        self.clipped_block_actors[name[0]] = clipped_actor

    self.setBounds()

    # Avoid z-buffer fighting
    vtk.vtkPolyDataMapper().SetResolveCoincidentTopologyToPolygonOffset()

    self.renderer.ResetCamera()
    self.vtkwidget.updateGL()
