import sys

def getMeshInfo(mesh_item_data):
  if 'file' in mesh_item_data:
    file_name = mesh_item_data['file']
    if '.e' in file_name:
      return ExodusIIMeshInfo(mesh_item_data, file_name)
  elif 'type' in mesh_item_data and mesh_item_data['type'] == 'GeneratedMesh':
    return GeneratedMeshInfo(mesh_item_data)
  else:
    return None

class MeshInfo:
  def __init__(self, mesh_item_data):
    self.mesh_item_data = mesh_item_data
    
  def blockNames(self):
    return []

  def sidesetNames(self):
    return []

class ExodusIIMeshInfo(MeshInfo):
  def __init__(self, mesh_item_data, file_name):
    MeshInfo.__init__(self, mesh_item_data)
    self.file_name = file_name
    import vtk
    reader = vtk.vtkExodusIIReader()
    reader.SetFileName(self.file_name)
    reader.UpdateInformation()
    num_sidesets = reader.GetNumberOfSideSetArrays()
    num_blocks = reader.GetNumberOfElementBlockArrays()

    self.sidesets = []
    for i in xrange(num_sidesets):
      self.sidesets.append(reader.GetObjectId(vtk.vtkExodusIIReader.SIDE_SET,i))
      if 'Unnamed' not in reader.GetObjectName(vtk.vtkExodusIIReader.SIDE_SET,i).split(' '):
        self.sidesets.append(reader.GetObjectName(vtk.vtkExodusIIReader.SIDE_SET,i).split(' ')[0])

    self.blocks = []
    for i in xrange(num_blocks):
      self.blocks.append(reader.GetObjectId(vtk.vtkExodusIIReader.ELEM_BLOCK,i))
      if 'Unnamed' not in reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' '):
        self.blocks.append(reader.GetObjectName(vtk.vtkExodusIIReader.ELEM_BLOCK,i).split(' ')[0])

  def blockNames(self):
    return self.blocks

  def sidesetNames(self):
    return self.sidesets

class GeneratedMeshInfo(MeshInfo):
  def __init__(self, mesh_item_data):
    MeshInfo.__init__(self, mesh_item_data)
    self.block_names = ['0']
    self.sideset_names = ['left','right','0','1']
    if 'dim' in self.mesh_item_data:
      if int(mesh_item_data['dim']) >= 2:
        self.sideset_names += ['bottom','top','2','3']
      if int(mesh_item_data['dim']) == 3:
        self.sideset_names += ['front','back','4','5']

  def blockNames(self):
    return self.block_names

  def sidesetNames(self):
    return self.sideset_names

