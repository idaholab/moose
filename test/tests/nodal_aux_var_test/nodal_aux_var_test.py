import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'nodal_aux_var_test.i',['out.e'], dofs, np, n_threads)

#def ts_test(dofs=0, np=0, n_threads=0):
#  tools.executeAppAndDiff(__file__,'nodal_aux_ts_test.i',['out_ts.e'], dofs, np, n_threads)

def multi_update_test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'multi_update_var_test.i',['out_multi_var.e'], dofs, np, n_threads)

  