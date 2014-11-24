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
  [./lc]
  [../]

  [./v1]
  [../]
  [./v2]
  [../]

  [./w1]
  [../]
[]

[AuxKernels]
  [./lc-aux]
    type = LinearCombinationAux
    variable = lc
    # error: the length has to match
    v = 'v1 v2'
    w = 'w1'
  [../]
[]


[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
