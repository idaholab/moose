import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'3d_penetration_locator_test.i',['out.e'], dofs, np, n_threads)

try: # temporary to test the new test harness
  from options import *

  test = { INPUT : '3d_penetration_locator_test.i',
           EXODIFF : ['out.e'] }

except:
  pass
