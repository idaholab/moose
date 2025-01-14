[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  uniform_refine = 2
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [v]
    order = FIRST
    family = LAGRANGE
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
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
  [top]
    type = DirichletBC
    variable = v
    boundary = 0
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = exodus_refined_restart_1
  exodus = true
[]
