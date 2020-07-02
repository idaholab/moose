# This is a mechanical constraint (contact formulation) version of glued_contact_mechanical_constraint.i
[Mesh]
  file = glued_contact_test.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./up]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0.5001'
  [../]

  [./lateral]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 1 0'
    scale_factor = 0.5
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 2
    secondary = 3
    penalty = 1e6
    model = glued
    formulation = kinematic
  [../]
[]

[BCs]
  [./bottom_lateral]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 1
    function = lateral
  [../]

  [./bottom_up]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1
    function = up
  [../]

  [./bottom_out]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./top]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./stiffStuff2]
    type = Elastic
    block = 2

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'ilu      101'

  line_search = 'none'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_tol = 1e-4

  l_max_its = 100
  nl_max_its = 10
  dt = 0.1
  num_steps = 30

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]
[]

[Postprocessors]
  active = ''
  [./resid]
    type = Residual
  [../]
  [./iters]
    type = NumNonlinearIterations
  [../]
[]

[Outputs]
  file_base = mechanical_constraint_out
  exodus = true
[]
