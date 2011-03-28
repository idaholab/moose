import tools

def testdirichletbc(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'forcing_function_test.i',['out.e'], dofs, np, n_threads)

def testneumannbc(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'forcing_function_neumannbc_test.i',['neumannbc_out.e'], dofs, np, n_threads)

