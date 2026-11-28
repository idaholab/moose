[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Problem]
  nl_sys_names = 'nl0 nl1'
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
  [coupled_uv]
    type = CoupledForce
    variable = u
    v = v
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
  [coupled_vu]
    type = CoupledForce
    variable = v
    v = u
  []
[]

[BCs]
  [left_u]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = ADDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = ADDirichletBC
    variable = v
    boundary = right
    value = 1
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
  [nmsm]
    type = NMSMExecutor
    sub_snes_executors = 'sub0 sub1'
  []
  [outer]
    type = NewtonSNESExecutor
    nl_preconditioning = nmsm
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
