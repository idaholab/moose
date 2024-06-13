# Since post-processors execute before controls on INITIAL,
# the first value in the gold file is 0 instead of 8.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [heat_fn]
    type = PiecewiseLinear
    x = '0 1 2 3 4 5'
    y = '8 -3.2 1.1 5.1 -1.4 0.3'
  []
[]

[Components]
[]

[ControlLogic]
  [input_value]
    type = GetFunctionValueControl
    function = heat_fn
  []
  [control_smoother]
    type = SmootherControl
    input = input_value:value
    n_points = 3
  []
[]

[Postprocessors]
  [control_value]
    type = RealControlDataValuePostprocessor
    control_data_name = control_smoother:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 5
[]

[Outputs]
  csv = true
[]
