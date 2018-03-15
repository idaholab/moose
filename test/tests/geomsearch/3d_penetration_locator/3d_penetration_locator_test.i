[Mesh]
  file = 3d_penetration_test.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
  [./tangential_distance]
    order = FIRST
    family = LAGRANGE
  [../]
  [./normal_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./normal_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./normal_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./closest_point_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./closest_point_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./closest_point_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./element_id]
    order = FIRST
    family = LAGRANGE
  [../]
  [./side]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]

  [./penetrate]
    type = PenetrationAux
    variable = penetration
    boundary = 2
    paired_boundary = 3
  [../]

  [./penetrate2]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
  [../]

  [./penetrate3]
    type = PenetrationAux
    variable = tangential_distance
    boundary = 2
    paired_boundary = 3
    quantity = tangential_distance
  [../]

  [./penetrate4]
    type = PenetrationAux
    variable = tangential_distance
    boundary = 3
    paired_boundary = 2
    quantity = tangential_distance
  [../]

  [./penetrate5]
    type = PenetrationAux
    variable = normal_x
    boundary = 2
    paired_boundary = 3
    quantity = normal_x
  [../]

  [./penetrate6]
    type = PenetrationAux
    variable = normal_x
    boundary = 3
    paired_boundary = 2
    quantity = normal_x
  [../]

  [./penetrate7]
    type = PenetrationAux
    variable = normal_y
    boundary = 2
    paired_boundary = 3
    quantity = normal_y
  [../]

  [./penetrate8]
    type = PenetrationAux
    variable = normal_y
    boundary = 3
    paired_boundary = 2
    quantity = normal_y
  [../]

  [./penetrate9]
    type = PenetrationAux
    variable = normal_z
    boundary = 2
    paired_boundary = 3
    quantity = normal_z
  [../]

  [./penetrate10]
    type = PenetrationAux
    variable = normal_z
    boundary = 3
    paired_boundary = 2
    quantity = normal_z
  [../]

  [./penetrate11]
    type = PenetrationAux
    variable = closest_point_x
    boundary = 2
    paired_boundary = 3
    quantity = closest_point_x
  [../]

  [./penetrate12]
    type = PenetrationAux
    variable = closest_point_x
    boundary = 3
    paired_boundary = 2
    quantity = closest_point_x
  [../]

  [./penetrate13]
    type = PenetrationAux
    variable = closest_point_y
    boundary = 2
    paired_boundary = 3
    quantity = closest_point_y
  [../]

  [./penetrate14]
    type = PenetrationAux
    variable = closest_point_y
    boundary = 3
    paired_boundary = 2
    quantity = closest_point_y
  [../]

  [./penetrate15]
    type = PenetrationAux
    variable = closest_point_z
    boundary = 2
    paired_boundary = 3
    quantity = closest_point_z
  [../]

  [./penetrate16]
    type = PenetrationAux
    variable = closest_point_z
    boundary = 3
    paired_boundary = 2
    quantity = closest_point_z
  [../]

  [./penetrate17]
    type = PenetrationAux
    variable = element_id
    boundary = 2
    paired_boundary = 3
    quantity = element_id
  [../]

  [./penetrate18]
    type = PenetrationAux
    variable = element_id
    boundary = 3
    paired_boundary = 2
    quantity = element_id
  [../]

  [./penetrate19]
    type = PenetrationAux
    variable = side
    boundary = 2
    paired_boundary = 3
    quantity = side
  [../]

  [./penetrate20]
    type = PenetrationAux
    variable = side
    boundary = 3
    paired_boundary = 2
    quantity = side
  [../]
[]

[BCs]
  active = 'block1_left block1_right block2_left block2_right'

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
  file_base = out
  exodus = true
[]
