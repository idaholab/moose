import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'displacement_test.i',['out_displaced_0001.e'], dofs, np)
