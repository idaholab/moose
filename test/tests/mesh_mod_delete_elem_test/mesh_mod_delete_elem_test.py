import tools

def test():
  pass
#  tools.executeAppAndDiff(__file__,'mesh_mod_delete_elem_test.i',['out.e'])

try:
  from options import *

  test = { INPUT : 'mesh_mod_delete_elem_test.i',
           EXODIFF : ['out.e'],
           SKIP : True }

except:
  pass
