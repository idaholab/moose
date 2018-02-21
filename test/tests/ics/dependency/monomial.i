[GlobalParams]
  family = MONOMIAL
  order = FIRST
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[AuxVariables]
  [./a]
  [../]

  [./b]
  [../]
[]

[Variables]
  [./u]
  [../]

  [./v]
  [../]
[]

[ICs]
  [./u_ic]
    type = ConstantIC
    variable = u
    value = -1
  [../]

  [./v_ic]
    type = MTICSum
    variable = v
    var1 = u
    var2 = a
  [../]

  [./a_ic]
    type = ConstantIC
    variable = a
    value = 10
  [../]

  [./b_ic]
    type = MTICMult
    variable = b
    var1 = v
    factor = 2
  [../]
[]

[AuxKernels]
  [./a_ak]
    type = ConstantAux
    variable = a
    value = 256
  [../]

  [./b_ak]
    type = ConstantAux
    variable = b
    value = 42
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

[BCs]
  [./left_u]
    type = PenaltyDirichletBC
    variable = u
    boundary = left
    value = 0
    penalty = 1000
  [../]

  [./right_u]
    type = PenaltyDirichletBC
    variable = u
    boundary = right
    value = 1
    penalty = 1000
  [../]


  [./left_v]
    type = PenaltyDirichletBC
    variable = v
    boundary = left
    value = 2
    penalty = 1000
  [../]

  [./right_v]
    type = PenaltyDirichletBC
    variable = v
    boundary = right
    value = 1
    penalty = 1000
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
