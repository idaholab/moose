[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = pl_test2.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./distance]
  [../]
  [./tangential_distance]
  [../]
  [./normal_x]
  [../]
  [./normal_y]
  [../]
  [./closest_point_x]
  [../]
  [./closest_point_y]
  [../]
  [./element_id]
  [../]
  [./side]
  [../]
[]

[Kernels]
  [./diff_x]
    type = Diffusion
    variable = disp_x
  [../]
  [./diff_y]
    type = Diffusion
    variable = disp_y
  [../]
[]

[AuxKernels]
  [./penetrate]
    type = PenetrationAux
    variable = distance
    boundary = 11            #secondary
    paired_boundary = 12     #primary
    tangential_tolerance = 0.1
  [../]

  [./penetrate2]
    type = PenetrationAux
    variable = distance
    boundary = 12            #secondary
    paired_boundary = 11     #primary
    tangential_tolerance = 0.1
  [../]

  [./penetrate3]
    type = PenetrationAux
    variable = tangential_distance
    boundary = 11
    paired_boundary = 12
    quantity = tangential_distance
  [../]

  [./penetrate4]
    type = PenetrationAux
    variable = tangential_distance
    boundary = 12
    paired_boundary = 11
    quantity = tangential_distance
  [../]

  [./penetrate5]
    type = PenetrationAux
    variable = normal_x
    boundary = 11
    paired_boundary = 12
    quantity = normal_x
  [../]

  [./penetrate6]
    type = PenetrationAux
    variable = normal_x
    boundary = 12
    paired_boundary = 11
    quantity = normal_x
  [../]

  [./penetrate7]
    type = PenetrationAux
    variable = normal_y
    boundary = 11
    paired_boundary = 12
    quantity = normal_y
  [../]

  [./penetrate8]
    type = PenetrationAux
    variable = normal_y
    boundary = 12
    paired_boundary = 11
    quantity = normal_y
  [../]

  [./penetrate9]
    type = PenetrationAux
    variable = closest_point_x
    boundary = 11
    paired_boundary = 12
    quantity = closest_point_x
  [../]

  [./penetrate10]
    type = PenetrationAux
    variable = closest_point_x
    boundary = 12
    paired_boundary = 11
    quantity = closest_point_x
  [../]

  [./penetrate11]
    type = PenetrationAux
    variable = closest_point_y
    boundary = 11
    paired_boundary = 12
    quantity = closest_point_y
  [../]

  [./penetrate12]
    type = PenetrationAux
    variable = closest_point_y
    boundary = 12
    paired_boundary = 11
    quantity = closest_point_y
  [../]

  [./penetrate13]
    type = PenetrationAux
    variable = element_id
    boundary = 11
    paired_boundary = 12
    quantity = element_id
  [../]

  [./penetrate14]
    type = PenetrationAux
    variable = element_id
    boundary = 12
    paired_boundary = 11
    quantity = element_id
  [../]

  [./penetrate15]
    type = PenetrationAux
    variable = side
    boundary = 11
    paired_boundary = 12
    quantity = side
  [../]

  [./penetrate16]
    type = PenetrationAux
    variable = side
    boundary = 12
    paired_boundary = 11
    quantity = side
  [../]
[]

[BCs]
  [./b1x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0
  [../]

  [./b1y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]

  [./b2x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0
  [../]

  [./b2y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = disp_y
  [../]
[]

[Functions]
  [./disp_y]
    type = PiecewiseLinear
    x = '0.0 0.25 0.75 1.0'
    y = '0.0 0.7 -0.7  0.0'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'

  nl_rel_tol = 1e-9
  l_max_its = 10

  start_time = 0.0
  dt = 0.05
  end_time = 1.0
[]

[Outputs]
  file_base = pl_test2tt_out
  exodus = true
[]
