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
    boundary = 3
    value = 1
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 2
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 2
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
