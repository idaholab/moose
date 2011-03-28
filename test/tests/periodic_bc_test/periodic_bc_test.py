import tools

def testperiodic(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'periodic_bc_test.i',['out.e'], dofs, np, n_threads)

def testwedge(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'wedge.i',['out_wedge.e'], dofs, np, n_threads)

def testwedgesys(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'wedge_sys.i',['out_wedge_sys.e'], dofs, np, n_threads)

def testtrapezoid(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'trapezoid.i',['out_trapezoid.e'], dofs, np, n_threads)
  
def testlevel1(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'periodic_level_1_test.i', \
                          ['level1_0000.e', 'level1_0009.e', 'level1_0019.e'], dofs, np, n_threads)
