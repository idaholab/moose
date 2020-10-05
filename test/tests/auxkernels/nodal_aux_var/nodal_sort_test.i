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
[]

[AuxVariables]
  [./one]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  [../]

  [./two]
    order = FIRST
    family = LAGRANGE
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
    boundary = '3 1'
    value = 5
  [../]

[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
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

  solve_type = 'PJFNK'
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
