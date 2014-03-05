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

[AuxBCs]
  active = ''

  [./five]
    type = ConstantAux
    variable = five
    boundary = '1 2'
    value = 5
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  console = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
    output_initial = true
  [../]
[]
