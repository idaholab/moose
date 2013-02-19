from GeneratedMeshInfo import GeneratedMeshInfo
from ExodusIIMeshInfo import ExodusIIMeshInfo

''' Factory Function '''
def getMeshInfo(mesh_item_data):
  if ( ('type' in mesh_item_data and mesh_item_data['type'] == 'FileMesh') or 'type' not in mesh_item_data) and 'file' in mesh_item_data and '.e' in mesh_item_data['file']:
    return ExodusIIMeshInfo(mesh_item_data, mesh_item_data['file'])
  else: # Everything else is handled by the MeshOnlyRenderer
    return ExodusIIMeshInfo(mesh_item_data, 'peacock_run_tmp_mesh.e') 
