[Mesh]
  file = rectangle.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./coupled_left]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]

  [./coupled_right]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]

  [./two]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 0
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    block = 1
    value = 10
  [../]
[]

[AuxKernels]
  [./coupled_left]
    variable = coupled_left
    type = CoupledAux
    value = 8
    operator = /
    coupled = two
  [../]

  [./coupled_right]
    variable = coupled_right
    type = CoupledAux
    value = 8
    operator = /
    coupled = two
  [../]

  [./two]
    type = ConstantAux
    variable = two
    value = 2
  [../]
[]

[BCs]
  active = 'right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
