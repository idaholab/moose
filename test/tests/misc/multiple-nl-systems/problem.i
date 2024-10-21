[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

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
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [force]
    type = CoupledForce
    variable = v
    v = u
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
  petsc_options = '-snes_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  first_nl_sys_to_solve = 'u'
  second_nl_sys_to_solve = 'v'
[]
