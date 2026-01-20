[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [test_fn]
    type = PiecewiseLinear
    x = '0    1   2  3  4     5'
    y = '100 10 9.5 11 10 1e-10'
  []
[]

[Postprocessors]
  [test_pp]
    type = FunctionValuePostprocessor
    function = test_fn
  []
[]

[Convergence]
  [test_conv]
    type = PostprocessorConvergence
    postprocessor = test_pp
    tolerance = 1e-7
    max_diverging_iterations = 2
    diverging_iteration_rel_reduction = 0.1
    verbose = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 1
  steady_state_detection = true
  steady_state_convergence = test_conv
[]
