import tools

def test_nodal_var_value(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'nodal_var_value.i',['out_nodal_var_value.e'], dofs, np, n_threads)

def test_nodal_aux_var_value(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'nodal_aux_var_value.i',['out_nodal_aux_var_value.e'], dofs, np, n_threads)

def test_avg_nodal_var_value(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'avg_nodal_var_value.i',['out_avg_nodal_var_value.e'], dofs, np, n_threads)
  
def test_inital(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'initial_pps.i',['out_initial_pps.e'], dofs, np, n_threads)

try:
  from options import *

  test_nodal_var_value = { INPUT : 'nodal_var_value.i',
                           EXODIFF : ['out_nodal_var_value.e'] }

  test_nodal_aux_var_value = { INPUT : 'nodal_aux_var_value.i',
                               EXODIFF : ['out_nodal_aux_var_value.e'] }

  test_avg_nodal_var_value = { INPUT : 'avg_nodal_var_value.i',
                               EXODIFF : ['out_avg_nodal_var_value.e'] }
  
  test_inital = { INPUT : 'initial_pps.i',
                  EXODIFF : ['out_initial_pps.e'] }

except:
  pass
