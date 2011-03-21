import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'nodal_aux_var_test.i',['out.e'], dofs, np)

#def ts_test(dofs=0, np=0):
#  tools.executeAppAndDiff(__file__,'nodal_aux_ts_test.i',['out_ts.e'], dofs, np)
  