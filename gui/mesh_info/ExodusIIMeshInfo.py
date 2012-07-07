from MeshInfo import *

''' Provides Information about ExodusII meshes '''
class ExodusIIMeshInfo(MeshInfo):
  def __init__(self, mesh_item_data, file_name):
    MeshInfo.__init__(self, mesh_item_data)
    self.file_name = file_name
    import vtk
    reader = vtk.vtkExodusIIReader()
    reader.SetFileName(self.file_name)
    reader.UpdateInformation()
    num_nodesets = reader.GetNumberOfNodeSetArrays()
    num_sidesets = reader.GetNumberOfSideSetArrays()
    num_blocks = reader.GetNumberOfElementBlockArrays()

    self.nodesets = set()
    for i in xrange(num_nodesets):
      self.nodesets.add(reader.GetObjectId(vtk.vtkExodusIIReader.NODE_SET,i))
      if 'Unnamed' not in reader.GetObjectName(vtk.vtkExodusIIReader.NODE_SET,i).split(' '):
        self.nodesets.add(reader.GetObjectName(vtk.vtkExodusIIReader.NODE_SET,i).split(' ')[0])

    self.sidesets = set()
    for i in xrange(num_sidesets):
      self.sidesets.add(reader.GetObjectId(vtk.vtkExodusIIReader.SIDE_SET,i))
      if 'Unnamed' not in reader.GetObjectName(vtk.vtkExodusIIReader.SIDE_SET,i).split(' '):
        self.sidesets.add(reader.GetObjectName(vtk.vtkExodusIIReader.SIDE_SET,i).split(' ')[0])

    self.blocks = set()
    for i in xrange(num_blocks):
      self.blocks.add(reader.GetObjectId(vtk.vtkExodusIIReader.ELEM_BLOCK,i))
      if 'Unnamed' not in reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' '):
        self.blocks.add(reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' ')[0])

  def blockNames(self):
    return self.blocks

  def sidesetNames(self):
    return self.sidesets

  def nodesetNames(self):
    return self.nodesets
