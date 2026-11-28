# Fixed point iterations over a linear system v and the nonlinear systems u and w
!include ../system_solve/base.i

[Convergence]
  # Later iterations start from a converged solution, so the solves need an absolute tolerance
  [nonlinear]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-12
  []
  # Used by the tests through the command line
  [two_iterations]
    type = IterationCountConvergence
    max_iterations = 2
    converge_at_max_iterations = true
  []
[]

[Executors]
  [steady]
    type = SteadyExecutor
    inner_executors = fixed_point
  []
  [fixed_point]
    type = FixedPointExecutor
    inner_executors = 'solve_v solve_u solve_w'
  []
  [solve_v]
    type = SystemSolveExecutor
    system = v_sys
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    l_tol = 1e-10
  []
  [solve_u]
    type = SystemSolveExecutor
    system = u_sys
    solve_type = NEWTON
    convergence = nonlinear
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    l_tol = 1e-10
  []
  [solve_w]
    type = SystemSolveExecutor
    system = w_sys
    solve_type = NEWTON
    convergence = nonlinear
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    l_tol = 1e-10
  []
[]
