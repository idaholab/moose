# Multiplicative Schwarz Preconditioned Inexact Newton (with backtracking)

!include physics.i

[Problem]
  nl_sys_names = 'u v'
[]

[Variables]
  [u]
    solver_sys = 'u'
  []
  [v]
    solver_sys = 'v'
  []
[]

[Kernels]
  [u_diff]
    extra_matrix_tags = 'NPC_J_0_1'
  []
[]

[ScalarKernels]
  [v_offdiag]
    extra_matrix_tags = 'NPC_J_1_0'
  []
[]

[BCs]
  [left]
    extra_matrix_tags = 'NPC_J_0_1'
  []
  [right]
    extra_matrix_tags = 'NPC_J_0_1'
  []
[]

[Executors]
  [newton]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'u v'
    nl_preconditioning = 'nmsm'
  []
  [steady]
    type = SteadyExecutor
    inner_executors = 'newton'
  []
  [nmsm]
    type = NMSMExecutor
    sub_snes_executors = 'u v'
  []
  [u]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'u'
    convergence_names = 'u'
    solve_type = Newton
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       mumps'
  []
  [v]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'v'
    convergence_names = 'v'
    solve_type = Newton
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       mumps'
  []
[]
