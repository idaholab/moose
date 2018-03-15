[Mesh]
  type = FileMesh
  file = 1d_penetration.e
  construct_side_list_from_node_list = true
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./penetration]
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
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 2
    paired_boundary = 3
  [../]
[]

[BCs]
  [./block1_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./block1_right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./block2_left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./block2_right]
    type = DirichletBC
    variable = u
    boundary = 4
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
