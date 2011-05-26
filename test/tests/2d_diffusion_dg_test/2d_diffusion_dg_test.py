import tools

def test(dofs=0, np=0, n_threads=0):
#  tools.executeAppAndDiff(__file__,'2d_diffusion_dg_test.i',['out_0000.e'], dofs, np, n_threads)
   pass       # fixme: when adding DG stuff into MOOSE-NG, uncomment this

try: # temporary to test the new test harness
  from options import *

  test = { INPUT : '2d_diffusion_dg_test.i',
           EXODIFF : ['out_0000.e'],
           SKIP : True }

except:
  pass
