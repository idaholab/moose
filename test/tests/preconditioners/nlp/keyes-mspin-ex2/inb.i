# INB stands for inexact Newton backtracking

!include physics.i

[Executors]
  [newton]
    type = NewtonSNESExecutor
    solve_type = Newton
    nonlinear_system_names = 'nl0'
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       mumps'
  []
  [steady]
    type = SteadyExecutor
    inner_executors = 'newton'
  []
[]
