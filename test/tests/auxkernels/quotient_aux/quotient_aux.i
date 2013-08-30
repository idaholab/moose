[Mesh]
  file = square.e
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

[AuxVariables]
  [./ratio]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[AuxKernels]
  [./ratio_auxkernel]
    type = QuotientAux
    variable = ratio
    numerator = u
    denominator = v
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 2
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 2
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
[]