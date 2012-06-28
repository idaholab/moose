import sys

def getMeshInfo(file_name):
  if '.e' in file_name:
    return ExodusIIMeshInfo(file_name)
  else:
    return None

class MeshInfo:
  def __init__(self, file_name):
    self.file_name = file_name
    
  def blockNames(self):
    return []

  def sidesetNames(self):
    return []


class ExodusIIMeshInfo(MeshInfo):
  def __init__(self, file_name):
    MeshInfo.__init__(self, file_name)
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

