import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'material_test.i',['out.e'], dofs, np)

def dg_test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'material_test_dg.i',['out_dg.e'], dofs, np)
  