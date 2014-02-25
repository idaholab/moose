import os, sys, PyQt4, getopt

from ExodusRenderer import ExodusRenderer
from MeshRenderer import MeshRenderer

class MeshOnlyRenderer(ExodusRenderer):
  def __init__(self, render_widget, mesh_item_data):
    #Not calling init for ExodusRenderer on purpose so we can override what it does
    MeshRenderer.__init__(self, render_widget, mesh_item_data)

    tmp_file_name = 'peacock_run_tmp_mesh.i'
    tmp_file = open(tmp_file_name,'w')
    tmp_file.write(render_widget.tree_widget.input_file_widget.input_file_textbox.buildInputString())
    tmp_file.close()

    os.system(render_widget.tree_widget.input_file_widget.app_path + ' -i peacock_run_tmp_mesh.i --mesh-only peacock_run_tmp_mesh.e > /dev/null')

    self.file_name = 'peacock_run_tmp_mesh.e'

    self.buildActors(self.file_name)
