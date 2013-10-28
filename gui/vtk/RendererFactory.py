from ExodusRenderer import ExodusRenderer
from MeshOnlyRenderer import MeshOnlyRenderer

''' Factory Function '''
def getRenderer(mesh_render_widget, mesh_item_data):
  if (('type' in mesh_item_data and mesh_item_data['type'] == 'FileMesh') \
      or 'type' not in mesh_item_data) and 'file' in mesh_item_data and '.e' in mesh_item_data['file']:

    # If we we are using Uniform Refine, we'll still need to use the MeshOnlyRenderer
    if 'uniform_refine' in mesh_item_data and int(mesh_item_data['uniform_refine']) > 0:
      return MeshOnlyRenderer(mesh_render_widget, mesh_item_data)
    else:
      return ExodusRenderer(mesh_render_widget, mesh_item_data)
  else:
    return MeshOnlyRenderer(mesh_render_widget, mesh_item_data)
