[Problem]
  nl_sys_names = 'nl0 nl1'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 5
[]

[Variables]
  [u]
    solver_sys = nl0
  []
  [v]
    solver_sys = nl1
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
  [coupled_uv]
    type = ADCoupledForce
    variable = u
    v = v
    matrix_tags = 'NPC_J_0_1'
  []
  [coupled_vu]
    type = ADCoupledForce
    variable = v
    v = u
    matrix_tags = 'NPC_J_1_0'
  []
[]

[BCs]
  [left_u]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
    extra_matrix_tags = 'NPC_J_0_1'
  []
  [right_u]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 1
    extra_matrix_tags = 'NPC_J_0_1'
  []
  [left_v]
    type = ADDirichletBC
    variable = v
    boundary = left
    value = 0
    extra_matrix_tags = 'NPC_J_1_0'
  []
  [right_v]
    type = ADDirichletBC
    variable = v
    boundary = right
    value = 1
    extra_matrix_tags = 'NPC_J_1_0'
  []
[]

[Convergence]
  [nl0]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
  [nl1]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
[]

[Executors]
  [shell_pc]
    type = NMSMExecutor
    sub_snes_executors = 'sub0 sub1'
  []
  [outer]
    type = NewtonSNESExecutor
    nl_preconditioning = shell_pc
    nonlinear_system_names = 'nl0 nl1'
  []
  [sub0]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl0'
    convergence_names = 'nl0'
  []
  [sub1]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl1'
    convergence_names = 'nl1'
  []
  [steady]
    type = SteadyExecutor
    inner_executors = 'outer'
  []
[]

[Outputs]
  console = false
[]
