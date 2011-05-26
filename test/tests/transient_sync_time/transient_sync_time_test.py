import tools

def testsynctimes(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'transient_sync_time_test.i',['out.e'], dofs, np, n_threads)

try:
  from options import *
  
  testsynctimes = { INPUT : 'transient_sync_time_test.i',
                    EXODIFF : ['out.e'] }

except:
  pass
