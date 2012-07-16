from GeneratedMeshInfo import GeneratedMeshInfo
from ExodusIIMeshInfo import ExodusIIMeshInfo

''' Factory Function '''
def getMeshInfo(mesh_item_data):
  print 'getMeshInfo', mesh_item_data['type']
  if 'file' in mesh_item_data:
    file_name = mesh_item_data['file']
    if '.e' in file_name:
      return ExodusIIMeshInfo(mesh_item_data, file_name)
  elif 'type' in mesh_item_data and mesh_item_data['type'] == 'GeneratedMesh':
    print 'In here!'
    return GeneratedMeshInfo(mesh_item_data)
  else:
    return None
