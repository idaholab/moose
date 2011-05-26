import tools

def global_param_test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'global_param_test.i',['out.e'], dofs, np, n_threads)

try:
  from options import *

  global_param_test = { INPUT : 'global_param_test.i',
                          EXODIFF : ['out.e'] }

except:
  pass
