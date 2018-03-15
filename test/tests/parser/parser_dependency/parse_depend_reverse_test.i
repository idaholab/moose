[Outputs]
  file_base = 2d_diffusion_reverse_out
  exodus = true
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Mesh]
  dim = 2
  file = square.e
[]
