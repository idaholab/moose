import tools

def test_steady(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'steady.i',['out_steady.e'], dofs, np, n_threads)

def test_steady_adapt(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'steady-adapt.i',['out_steady_adapt_0003.e'], dofs, np, n_threads)

def test_transient(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'transient.i',['out_transient.e'], dofs, np, n_threads)
  
def test_sln_time_adapt(dofs=0, np=0, n_threads=0):
  pass
#  tools.executeAppAndDiff(__file__,'sln-time-adapt.i',['out_sta.e'], dofs, np, n_threads)

try:
  from options import *

  test_steady = { INPUT : 'steady.i',
                 EXODIFF : ['out_steady.e'] }

  test_steady_adapt = { INPUT : 'steady-adapt.i',
                      EXODIFF : ['out_steady_adapt_0003.e'] }

  test_transient = { INPUT : 'transient.i',
                    EXODIFF : ['out_transient.e'] }

  test_sln_time_adapt = { INPUT : 'sln-time-adapt.i',
                          EXODIFF : ['out_sta.e'] ,
                          SKIP : True }

except:
  pass
