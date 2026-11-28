# Executor version of the relaxed fixed point iteration of a linear advection system
!include ../../linearfvkernels/advection/diagonal-step-2d.i

inactive = 'Executioner'

[Executors]
  [steady]
    type = SteadyExecutor
    inner_executors = fixed_point
  []
  [fixed_point]
    type = FixedPointExecutor
    inner_executors = solve_u
    convergence = linear
  []
  [solve_u]
    type = SystemSolveExecutor
    system = u_sys
    relaxation_factor = 0.3
    l_tol = 1e-12
    petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -pc_factor_mat_solver_type -mat_mumps_icntl_14 -mat_mumps_icntl_20'
    petsc_options_value = 'lu       NONZERO               1e-12                     mumps                    50                  0'
  []
[]

[Outputs]
  file_base = minmod
[]
