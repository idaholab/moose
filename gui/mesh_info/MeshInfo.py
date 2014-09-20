import sys

''' A base class for providing information about the currently chosen mesh '''
class MeshInfo:
  def __init__(self, mesh_item_data):
    self.mesh_item_data = mesh_item_data

  def blockNames(self):
    return set()

  def sidesetNames(self):
    return set()

  def nodesetNames(self):
    return set()
