import tools

def test_nodal_var_value(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'nodal_var_value.i',['out_nodal_var_value.e'], dofs, np)

def test_nodal_aux_var_value(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'nodal_aux_var_value.i',['out_nodal_aux_var_value.e'], dofs, np)

def test_avg_nodal_var_value(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'avg_nodal_var_value.i',['out_avg_nodal_var_value.e'], dofs, np)
  