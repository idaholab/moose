import tools

def test_supg(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'supg.i',['out_supg.e'], dofs, np, n_threads)

