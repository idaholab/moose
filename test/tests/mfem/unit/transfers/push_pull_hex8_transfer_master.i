[Mesh]
  type = CoupledMFEMMesh
  file = gold/simple-cube-hex8.e
  dim = 3
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [sent_variable]
    family = LAGRANGE
    order = FIRST
  []

  [received_variable]
    family = LAGRANGE
    order = FIRST
  []
[]

[ICs]
  [set_variable]
    type = FunctionIC
    variable = sent_variable
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
    variable = sent_variable
    other_variable = received_variable
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = 'push_pull_hex8_transfer_sub_app.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [push]
    type = MultiAppCopyTransfer
    to_multi_app = sub_app
    source_variable = sent_variable
    variable = received_variable_subapp
  []

  [pull]
    type = MultiAppCopyTransfer
    from_multi_app = sub_app
    source_variable = received_variable_subapp
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