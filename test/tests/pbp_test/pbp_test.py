import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'pbp_test.i',['out.e'], dofs, np, n_threads)
