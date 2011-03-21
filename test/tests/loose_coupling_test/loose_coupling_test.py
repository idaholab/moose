import tools

def test_same(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'loose-coupling-same.i',['out_same.e'], dofs, np)

