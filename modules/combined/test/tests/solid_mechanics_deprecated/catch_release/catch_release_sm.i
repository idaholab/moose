[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = catch_release.e
[]

[Functions]
  [./up]
    type = PiecewiseLinear
    x = '0 1 2.00 3 4'
    y = '0 1 1.01 1 0'
    scale_factor = 0.5
  [../]
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 2
    secondary = 3
    penalty = 1e6
    model = frictionless
  [../]
[]

[BCs]
  [./lateral]
    type = DirichletBC
    variable = disp_x
    boundary = '1 4'
    value = 0.0
  [../]

  [./bottom_up]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = 1
    function = up
  [../]

  [./out]
    type = DirichletBC
    variable = disp_z
    boundary = '1 4'
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

  #petsc_options_iname = '-pc_type -snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart'
  #petsc_options_value = 'ilu      ls         basic    basic                    101'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      101'

  line_search = 'none'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-4
  l_tol = 1e-4

  l_max_its = 100
  nl_max_its = 20
  dt = 1.0
  end_time = 4.0
[]

[Outputs]
  exodus = true
[]
