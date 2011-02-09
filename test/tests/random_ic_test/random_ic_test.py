import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'random_ic_test.i',['out.e'], dofs, np)
