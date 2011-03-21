import tools

def test():
  tools.executeAppAndDiff(__file__,'mesh_mod_delete_elem_test.i',['out.e'])

