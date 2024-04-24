[Mesh]
  type = CoupledMFEMMesh
  file = gold/simple-cube-tet4.e
  dim = 3
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [received_variable]
    family = LAGRANGE
    order = FIRST
  []

  [expected_variable]
    family = LAGRANGE
    order = FIRST
  []
[]

[ICs]
  [set_variable]
    type = FunctionIC
    variable = expected_variable
    function = set_variable
  []
[]  

[Functions]
  [set_variable]
    type = ParsedFunction
    expression = '42 + 100*x*x'
  []
[]

[Postprocessors]
  [l2_difference]
    type = ElementL2Difference
    variable = expected_variable
    other_variable = received_variable
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = 'pull_tet4_transfer_sub_app.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [pull]
    type = MultiAppCopyTransfer
    from_multi_app = sub_app
    source_variable = sent_variable
    variable = received_variable
  []  
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  csv = true
[]