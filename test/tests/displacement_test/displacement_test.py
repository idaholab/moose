import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'displacement_test.i',['out_displaced_0001.e'], dofs, np, n_threads)

try:
  from options import *

  test = { INPUT : 'displacement_test.i',
           EXODIFF : ['out_displaced_0001.e'] }

except:
  pass
