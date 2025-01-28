[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Functions]
  [test_fn]
    type = ConstantFunction
    value = 0
  []
[]

[Postprocessors]
  [test_pp]
    type = ConstantPostprocessor
    value = -1e-6
  []
  [num_nl_iterations]
    type = NumNonlinearIterations
    execute_on = 'TIMESTEP_END'
  []
[]

[Convergence]
  [parsed_conv]
    type = ParsedConvergence
    convergence_expression = 'fn | (supplied & abs(pp) < tol)'
    symbol_names = 'supplied pp tol fn'
    symbol_values = 'supplied_conv test_pp 1e-5 test_fn'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  nonlinear_convergence = parsed_conv
[]

[Outputs]
  csv = true
  show = 'num_nl_iterations'
  execute_on = 'FINAL'
[]
