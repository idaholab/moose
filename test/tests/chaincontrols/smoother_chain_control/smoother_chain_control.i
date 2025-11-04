# Tests SmootherChainControl
#
# Note that since post-processors execute before controls on INITIAL,
# the first value in the gold file is 0.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [test_fn]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '5 3 7 2' # corresponding 3-point averages are: 5, 4, 5, 4
  []
[]

[ChainControls]
  [input_ctrl]
    type = GetFunctionValueChainControl
    function = test_fn
  []
  [smoother_ctrl]
    type = SmootherChainControl
    input = input_ctrl:value
    n_points = 3
  []
[]

[Postprocessors]
  [smoothed_value]
    type = ChainControlDataPostprocessor
    chain_control_data_name = smoother_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
[]

[Outputs]
  csv = true
[]
