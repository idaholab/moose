import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'stateful_prop_test.i',['out.e'], dofs, np, n_threads)

def spatial_test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'stateful_prop_spatial_test.i',['out_spatial.e'], dofs, np, n_threads)
  