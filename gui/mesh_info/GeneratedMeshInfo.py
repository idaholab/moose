from MeshInfo import *

''' Provides information about Generated Meshes '''
class GeneratedMeshInfo(MeshInfo):
  def __init__(self, mesh_item_data):
    MeshInfo.__init__(self, mesh_item_data)
    self.block_names = set(['0'])
    self.sideset_names = set(['left','right','0','1'])
    if 'dim' in self.mesh_item_data:
      if int(mesh_item_data['dim']) >= 2:
        self.sideset_names |= set(['bottom','top','2','3'])
      if int(mesh_item_data['dim']) == 3:
        self.sideset_names |= set(['front','back','4','5'])

    self.nodeset_names = set()

  def blockNames(self):
    return self.block_names

  def sidesetNames(self):
    return self.sideset_names

  def nodesetNames(self):
    return self.nodeset_names
