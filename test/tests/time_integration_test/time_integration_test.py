import tools

def testie(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'ie.i',['out_ie.e'], dofs, np)
  
def testbdf2(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'bdf2.i',['out_bdf2.e'], dofs, np)

def testcranic(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'cranic.i',['out_cranic.e'], dofs, np)


def testdt2(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'dt2.i',['out_dt2.e'], dofs, np)
