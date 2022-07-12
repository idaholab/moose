[Mesh]
  file = sq-2blk.e
  uniform_refine = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./u_aux]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./ic_u_1]
    type = ConstantIC
    variable = u
    value = 42
    block = '1'
  [../]
  [./ic_u_2]
    type = ConstantIC
    variable = u
    value = 24
    #  Oops - can't have two ICs on the same block
  [../]
  [./ic_u_aux_1]
    type = ConstantIC
    variable = u_aux
    value = 6.25
    block = '1'
  [../]
  [./ic_u_aux_2]
    type = ConstantIC
    variable = u_aux
    value = 9.99
    block = '2'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
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

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
[]
