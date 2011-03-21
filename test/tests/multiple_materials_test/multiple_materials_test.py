import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'multiple_materials_test.i',['out.e'], dofs, np)
