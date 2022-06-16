[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]

  [./v]
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
    coeff = 1
  [../]

  [./body_force]
    type = FVBodyForce
    variable = u
    value = 10
  [../]
[]

[FVBCs]
  active = 'right'

  [./left]
    type = FVDirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./right]
    type = FVDirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = out
[]
