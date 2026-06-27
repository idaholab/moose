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
    value = 1
  []
[]

[Executioner]
  type = SteadySolve2
  solve_type = 'NEWTON'
  first_nl_sys_to_solve = 'u_sys'
  second_nl_sys_to_solve = 'v_sys'
[]

[Postprocessors]
  [num_dofs_u_sys]
    type = NumDOFs
    system = u_sys
  []
  [num_dofs_v_sys]
    type = NumDOFs
    system = v_sys
  []
  [num_dofs_all]
    type = NumDOFs
  []
[]

[Outputs]
  csv = true
[]
