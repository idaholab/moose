[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Problem]
  nl_sys_names = 'u_sys v_sys'
[]

[Variables]
  [u]
    solver_sys = 'u_sys'
  []
  [v]
    solver_sys = 'v_sys'
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 2
  []
[]

[Postprocessors]
  [u_sys_pre_smo]
    type = Residual
    residual_type = PRE_SMO
    solver_sys = u_sys
  []
  [v_sys_pre_smo]
    type = Residual
    residual_type = PRE_SMO
    solver_sys = v_sys
  []
  [u_sys_initial]
    type = Residual
    residual_type = INITIAL
    solver_sys = u_sys
  []
  [v_sys_initial]
    type = Residual
    residual_type = INITIAL
    solver_sys = v_sys
  []
[]

[Executioner]
  type = SteadySolve2
  solve_type = 'NEWTON'
  first_nl_sys_to_solve = 'u_sys'
  second_nl_sys_to_solve = 'v_sys'
  use_pre_SMO_residual = true
[]

[Outputs]
  csv = true
[]
