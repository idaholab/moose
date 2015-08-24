[Mesh]
  file = square.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./one]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 0
  [../]

  [./two]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  # Intentionally out of order to test sorting capabiilties
  active = 'one two'
  [./two]
    variable = two
    type = CoupledAux
    value = 2
    operator = '/'
    coupled = one
  [../]

  [./one]
    variable = one
    type = ConstantAux
    value = 1
  [../]

  [./five]
    type = ConstantAux
    variable = five
    boundary = '1 2'
    value = 5
  [../]
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

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
