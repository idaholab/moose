[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [sent_vector]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Functions]
  [set_sent_vector]
    type = ParsedVectorFunction
    expression_x = '100*x*x'
    expression_y = '100*y*y'
    expression_z = '100*z*z'
  []
[]

[ICs]
  # Set sent vector. We will check this in the subapp.
  [set_sent_vector]
    type = VectorFunctionIC
    variable = sent_vector
    function = set_sent_vector
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = 'sub_L2_LagrangeVec.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [push]
    type = MultiAppCopyTransfer
    to_multi_app = sub_app
    source_variable = 'sent_vector'
    variable = 'received_vector'
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]
