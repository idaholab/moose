# pp1 converges (< 1e-6) at t=2
# pp2 converges (< 1e-7) at t=3
# Therefore, all converge at t=3, so expected number of dt=1 steps is 3.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [pp1_fn]
    type = PiecewiseLinear
    x = '0    1    2    3    4    5'
    y = '1e-6 1e-3 1e-6 1e-7 1e-8 1e-9'
  []
  [pp2_fn]
    type = PiecewiseLinear
    x = '0    1    2    3    4    5'
    y = '1e-8 1e-6 1e-6 1e-8 1e-9 1e-9'
  []
[]

[Postprocessors]
  [pp1]
    type = FunctionValuePostprocessor
    function = pp1_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [pp2]
    type = FunctionValuePostprocessor
    function = pp2_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Convergence]
  [ss_conv_with_descriptions]
    type = MultiPostprocessorConvergence
    postprocessors = 'pp1 pp2'
    descriptions = 'QuantityA QuantityB'
    tolerances = '1e-5 1e-7'
  []
  [ss_conv_without_descriptions]
    type = MultiPostprocessorConvergence
    postprocessors = 'pp1 pp2'
    tolerances = '1e-5 1e-7'
  []
  [ss_conv]
    type = ParsedConvergence
    convergence_expression = 'conv1 & conv2'
    symbol_names = 'conv1 conv2'
    symbol_values = 'ss_conv_with_descriptions ss_conv_without_descriptions'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 5
  steady_state_detection = true
  steady_state_convergence = ss_conv
  verbose = true
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
