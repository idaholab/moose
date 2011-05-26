import tools

def testmeta(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'meta_action_test.i',['out.e'], dofs, np, n_threads)

try:
  from options import *

  testmeta = { INPUT : 'meta_action_test.i',
               EXODIFF : ['out.e'] }

except:
  pass
