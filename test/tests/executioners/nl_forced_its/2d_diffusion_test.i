[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  # BCs cannot be preset due to Jacobian test
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  nl_forced_its = 2
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-50
  solve_type = 'NEWTON'
[]
