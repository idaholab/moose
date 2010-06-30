import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'parse_depend_mixed_test.i',['2d_diffusion_out.e'], dofs, np)

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'parse_depend_reverse_test.i',['2d_diffusion_out.e'], dofs, np)

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'parse_depend_pbp_test.i',['pbp_out.e'], dofs, np)
