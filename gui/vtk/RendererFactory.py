from ExodusRenderer import ExodusRenderer
from MeshOnlyRenderer import MeshOnlyRenderer

''' Factory Function '''
def getRenderer(mesh_render_widget, mesh_item_data):
  if ( ('type' in mesh_item_data and mesh_item_data['type'] == 'MooseMesh') or 'type' not in mesh_item_data) and 'file' in mesh_item_data and '.e' in mesh_item_data['file']:
    return ExodusRenderer(mesh_render_widget, mesh_item_data)
  else:
    return MeshOnlyRenderer(mesh_render_widget, mesh_item_data)
