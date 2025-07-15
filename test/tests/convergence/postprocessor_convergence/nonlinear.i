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

[AuxVariables]
  [w]
  []
[]

[AuxKernels]
  [w_kernel]
    type = ParsedAux
    variable = w
    expression = 'sol'
    functor_names = 'u'
    functor_symbols = 'sol'
    execute_on = 'NONLINEAR_CONVERGENCE'
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

[Postprocessors]
  [w_change]
    type = AverageVariableChange
    variable = w
    change_over = nonlinear_iteration
    norm = L1
    execute_on = 'NONLINEAR_CONVERGENCE'
  []
  [num_nl_iterations]
    type = NumNonlinearIterations
    execute_on = 'TIMESTEP_END'
  []
[]

[Convergence]
  [test_conv]
    type = PostprocessorConvergence
    postprocessor = w_change
    tolerance = 1e-7
  []
[]

[Problem]
  previous_nl_solution_required = true
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  nonlinear_convergence = test_conv
[]

[Outputs]
  csv = true
  show = 'num_nl_iterations'
  execute_on = 'FINAL'
[]
