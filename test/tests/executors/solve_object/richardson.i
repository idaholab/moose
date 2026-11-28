# Fixed point iterations over a linear system v and the nonlinear systems u and w. A Richardson
# iteration of a solve object solves u.
!include ../system_solve/base.i

[Convergence]
  # Later iterations start from a converged solution, so the solves need an absolute tolerance
  [nonlinear]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-12
  []
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
    inner_executors = 'solve_v richardson_u solve_w'
    convergence = two_iterations
  []
  [solve_v]
    type = SystemSolveExecutor
    system = v_sys
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    l_tol = 1e-10
  []
  [richardson_u]
    type = RichardsonTestSolveExecutor
    # 2 / (smallest + largest eigenvalue of the Jacobian of u on this mesh), the optimal step size
    step_size = 0.25
    richardson_abs_tol = 1e-12
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
