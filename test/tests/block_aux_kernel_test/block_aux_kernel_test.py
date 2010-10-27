import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'block_aux_kernel_test.i',['out.e'], dofs, np)
