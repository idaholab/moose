import tools

def testperiodic(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'periodic_bc_test.i',['out.e'], dofs, np)

def testwedge(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'wedge.i',['out_wedge.e'], dofs, np)

def testtrapezoid(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'trapezoid.i',['out_trapezoid.e'], dofs, np)
  
