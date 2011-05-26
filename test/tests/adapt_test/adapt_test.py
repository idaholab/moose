import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'adapt_test.i',['out_0001.e'], dofs, np, n_threads)

def test_time(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'adapt_time_test.i',['out_time_0006.e'], dofs, np, n_threads)
  

try: # temporary to test the new test harness
  from options import *

  test = { INPUT : 'adapt_test.i',
           EXODIFF : ['out_0001.e'] }

  test_time   = { INPUT : 'adapt_time_test.i',
                  EXODIFF : ['out_time_0006.e'] }

except:
  pass
