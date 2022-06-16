[Mesh]
  file = rectangle.e
[]

[Variables]
  active = 'u'

  [./u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
[]

[FVKernels]
  active = 'diff body_force'

  [./diff]
    type = FVDiffusion
    variable = u
    block = 1
    coeff = 1
  [../]

  [./body_force]
    type = FVBodyForce
    variable = u
    block = 1
    value = 10
  [../]
[]

[FVBCs]
  active = 'right'

  [./left]
    type = FVDirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right]
    type = FVDirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
