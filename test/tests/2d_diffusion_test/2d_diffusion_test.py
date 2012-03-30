from options import *

testdirichlet = { INPUT : '2d_diffusion_test.i',
                  EXODIFF : ['out.e'],
                  SCALE_REFINE : 5
                  }

testneumann   = { INPUT : '2d_diffusion_neumannbc_test.i',
                  EXODIFF : ['neumannbc_out.e'],
                  SCALE_REFINE : 5
                  }



