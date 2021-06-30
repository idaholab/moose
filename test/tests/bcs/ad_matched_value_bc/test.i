[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

# Solves a pair of coupled diffusion equations where u=v on the boundary

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 3
  []

  [v]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2
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
[]

[BCs]
  [right_v]
    type = ADDirichletBC
    variable = v
    boundary = 1
    value = 3
  []

  [left_u]
    type = ADMatchedValueBC
    variable = u
    boundary = 3
    v = v
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  nl_rel_tol = 1e-10
  l_tol = 1e-12
[]

[Outputs]
  file_base = out
  exodus = true
[]
