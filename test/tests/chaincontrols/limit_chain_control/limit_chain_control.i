# Tests LimitChainControl

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [test_fn]
    type = ParsedFunction
    expression = 'sin(3*pi*t+1)'
  []
[]

[Postprocessors]
  [unlimited_value]
    type = ChainControlDataPostprocessor
    chain_control_data_name = unlimited_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [limited_value]
    type = ChainControlDataPostprocessor
    chain_control_data_name = limited_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ChainControls]
  [unlimited_ctrl]
    type = GetFunctionValueChainControl
    function = test_fn
    point = '0 0 0'
  []
  [limited_ctrl]
    type = LimitChainControl
    control_data = unlimited_ctrl:value
    min_value = -0.2
    max_value = 0.6
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  csv = true
[]
