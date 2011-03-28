import tools

def testdirichlet(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'constant_damper_test.i',['out.e'], dofs, np, n_threads)
