# Tests PIDChainControl
#
# The PID controller is used to solve the equation y(alpha) = alpha^2 = 25 for alpha.
# Each time step takes one iteration, and these iterations should be converging to alpha = 5.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[ChainControls]
  [get_alpha_ctrl]
    type = GetPostprocessorChainControl
    postprocessor = pid
  []
  [y_ctrl]
    type = ParsedChainControl
    expression = 'alpha^2'
    symbol_names = 'alpha'
    symbol_values = 'get_alpha_ctrl:value'
  []
  [y_set_ctrl]
    type = ParsedChainControl
    expression = '25'
  []
  [pid_ctrl]
    type = PIDChainControl
    input = y_ctrl:value
    set_point = y_set_ctrl:value
    K_p = 0.1
    K_i = 0.01
    K_d = 0.001
  []
[]

[Postprocessors]
  [pid]
    type = ChainControlDataPostprocessor
    chain_control_data_name = pid_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 50
[]

[Outputs]
  csv = true
[]
