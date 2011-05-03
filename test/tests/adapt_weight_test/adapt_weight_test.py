import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'adapt_weight_test.i',['out_0001.e'], dofs, np, n_threads)
