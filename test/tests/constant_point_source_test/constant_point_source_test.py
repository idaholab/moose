import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'constant_point_source_test.i',['out.e'], dofs, np, n_threads)

