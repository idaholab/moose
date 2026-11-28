!include base.i

[Convergence]
  # Used by the tests through the command line
  # Accepts the initial guess without an iteration
  [no_iterations]
    type = IterationCountConvergence
    max_iterations = 0
    converge_at_max_iterations = true
    verbose = true
  []
[]

[Executors]
  [steady]
    type = SteadyExecutor
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
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    l_tol = 1e-10
  []
  [solve_w]
    type = SystemSolveExecutor
    system = w_sys
    solve_type = NEWTON
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    l_tol = 1e-10
  []
[]
