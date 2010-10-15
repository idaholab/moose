import tools

def testperiodic(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'periodic_bc_test.i',['out.e'], dofs, np)

