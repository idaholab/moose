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
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff body_force'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    value = 10
  [../]
[]

[BCs]
  active = 'right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./right]
    type = DirichletBC
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
