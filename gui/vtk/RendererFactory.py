from ExodusRenderer import ExodusRenderer
from GeneratedMeshRenderer import GeneratedMeshRenderer

''' Factory Function '''
def getRenderer(mesh_render_widget, mesh_item_data):
  if 'type' in mesh_item_data and mesh_item_data['type'] == 'GeneratedMesh':
    return GeneratedMeshRenderer(mesh_render_widget, mesh_item_data)
  elif 'file' in mesh_item_data:
    file_name = mesh_item_data['file']
    if '.e' in file_name:
      return ExodusRenderer(mesh_render_widget, mesh_item_data)
  else:
    return None
