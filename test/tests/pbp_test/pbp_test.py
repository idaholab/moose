import tools

def test(dofs=0):
  tools.executeAppAndDiff(__file__,'pbp_test.i',['out.e'], dofs)
