import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'material_test.i',['out.e'], dofs, np, n_threads)

#def dg_test(dofs=0, np=0, n_threads=0):
#  tools.executeAppAndDiff(__file__,'material_test_dg.i',['out_dg.e'], dofs, np, n_threads)

def coupled_material_test(dofs=0, np=0, n_threads=0):
  pass
#  tools.executeAppAndDiff(__file__,'coupled_material_test.i',['out_coupled.e'], dofs, np, n_threads)
