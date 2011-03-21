import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'coupled_bc_test.i',['out.e'], dofs, np)
