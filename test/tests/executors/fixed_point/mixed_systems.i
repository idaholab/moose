# Executor version of the fixed point iteration between a nonlinear and a linear system
!include ../../multisystem/picard/linearfv_nonlinearfv/same_input.i

inactive = 'Executioner'

[Convergence]
  [nonlinear]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
  []
[]

[Executors]
  [steady]
    type = SteadyExecutor
    inner_executors = fixed_point
  []
  [fixed_point]
    type = FixedPointExecutor
    inner_executors = 'solve_v solve_u'
    convergence = linear
  []
  [solve_v]
    type = SystemSolveExecutor
    system = v_sys
    convergence = nonlinear
    l_abs_tol = 1e-12
    l_tol = 1e-10
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
  []
  [solve_u]
    type = SystemSolveExecutor
    system = u_sys
    l_abs_tol = 1e-12
    l_tol = 1e-10
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
  []
[]

[Outputs]
  file_base = same_input_out
[]
