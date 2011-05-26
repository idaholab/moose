import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'side_average_value_test.i',['out.e'], dofs, np, n_threads)

try:
  from options import *

  test = { INPUT : 'side_average_value_test.i',
           EXODIFF : ['out.e'] }

except:
  pass
