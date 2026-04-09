[Problem]
  type = MFEMProblem
  solve = false
[]

[Mesh]
  type = MFEMMesh
  file = ../../mesh/square_quad.e
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[AuxVariables]
  [sent_nodal]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [received_nodal]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [sent_elem]
    type = MFEMVariable
    fespace = L2FESpace
  []
  [received_elem]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [sent_nodal_var_func]
    type = ParsedFunction
    expression = '1 + 2*x*x + 3*y*y*y'
  []
  [sent_elem_var_func]
    type = ParsedFunction
    expression = '2 + 2*x*x + 3*y*y*y'
  []
  [received_nodal_var_func]
    type = ParsedFunction
    expression = '3 + 2*x*x + 3*y*y*y'
  []
  [received_elem_var_func]
    type = ParsedFunction
    expression = '4 + 2*x*x + 3*y*y*y'
  []
[]

[ICs]
  [sent_nodal_var_ic]
    type = MFEMScalarIC
    variable = 'sent_nodal'
    coefficient = sent_nodal_var_func
  []
  [sent_elem_var_ic]
    type = MFEMScalarIC
    variable = 'sent_elem'
    coefficient = sent_elem_var_func
  []
  [received_nodal_var_ic]
    type = MFEMScalarIC
    variable = 'received_nodal'
    coefficient = -1
  []
  [received_elem_var_ic]
    type = MFEMScalarIC
    variable = 'received_elem'
    coefficient = -1
  []

[]

[Executioner]
  type = MFEMTransient
  num_steps = 1
  device = cpu
[]

[VectorPostprocessors]
  [nodal_sample]
    type = MFEMLineValueSampler
    variable = 'received_nodal'
    start_point = '0.0 0.0 0.0'
    end_point = '1.0 1.0 0.0'
    num_points = 14
    execute_on = TIMESTEP_END
  []
  [elem_sample]
    type = MFEMLineValueSampler
    variable = 'received_elem'
    start_point = '0.0 0.0 0.0'
    end_point = '1.0 1.0 0.0'
    num_points = 14
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  csv = true
[]
