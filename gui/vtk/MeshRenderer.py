''' Base class for how to render a mesh '''
class MeshRenderer:
  def __init__(self, render_widget, mesh_item_data):
    self.render_widget = render_widget
    self.renderer = self.render_widget.renderer
    self.vtkwidget = self.render_widget.vtkwidget

    self.mesh_item_data = mesh_item_data

    self.plane = render_widget.plane

    # These need to get filled in by the children:
    self.all_actors = []

    self.block_actors = {}
    self.clipped_block_actors = {}

    self.sideset_actors = {}
    self.clipped_sideset_actors = {}

    self.nodeset_actors = {}
    self.clipped_nodeset_actors = {}



  def setFileName(self, file_name):
    raise NotImplementedError
