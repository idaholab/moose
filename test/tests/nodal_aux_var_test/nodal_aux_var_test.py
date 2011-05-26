import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'nodal_aux_var_test.i',['out.e'], dofs, np, n_threads)

#def ts_test(dofs=0, np=0, n_threads=0):
#  tools.executeAppAndDiff(__file__,'nodal_aux_ts_test.i',['out_ts.e'], dofs, np, n_threads)

def multi_update_test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'multi_update_var_test.i',['out_multi_var.e'], dofs, np, n_threads)

try:
  from options import *

  test = { INPUT : 'nodal_aux_var_test.i',
           EXODIFF : ['out.e'] }

  ts_test = { INPUT : 'nodal_aux_ts_test.i',
              EXODIFF : ['out_ts.e'],
              SKIP : True }

  multi_update_test = { INPUT : 'multi_update_var_test.i',
                        EXODIFF : ['out_multi_var.e'] }

except:
  pass
