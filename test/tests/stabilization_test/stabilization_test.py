import tools

def test_supg(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'supg.i',['supg_out.e'], dofs, np)

