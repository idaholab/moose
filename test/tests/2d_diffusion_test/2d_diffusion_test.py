import tools

def testdirichlet(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'2d_diffusion_test.i',['out.e'], dofs, np)

def testneumann(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'2d_diffusion_neumannbc_test.i',['neumannbc_out.e'], dofs, np)
