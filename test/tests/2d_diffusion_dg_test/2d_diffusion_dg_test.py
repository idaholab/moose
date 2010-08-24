import tools

def test():
  tools.executeAppAndDiff(__file__,'2d_diffusion_dg_test.i',['out_0000.e'])

