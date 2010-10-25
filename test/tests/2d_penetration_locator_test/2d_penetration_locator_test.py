import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'2d_penetration_locator_test.i',['out.e'], dofs, np)
