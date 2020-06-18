[GlobalParams]
  volumetric_locking_correction = false
[]

[Mesh]
  file = merged.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]

  [./disp_y]
  [../]

  [./disp_z]
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_zx]
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

[DiracKernels]
  [./primary_x]
    type = ContactPrimary
    variable = disp_x
    component = 0
    boundary = 3
    secondary = 2
  [../]

  [./primary_y]
    type = ContactPrimary
    variable = disp_y
    component = 1
    boundary = 3
    secondary = 2
  [../]

  [./primary_z]
    type = ContactPrimary
    variable = disp_z
    component = 2
    boundary = 3
    secondary = 2
  [../]

  [./secondary_x]
    type = SecondaryConstraint
    variable = disp_x
    component = 0
    boundary = 2
    primary = 3
  [../]

  [./secondary_y]
    type = SecondaryConstraint
    variable = disp_y
    component = 1
    boundary = 2
    primary = 3
  [../]

  [./secondary_z]
    type = SecondaryConstraint
    variable = disp_z
    component = 2
    boundary = 2
    primary = 3
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]

  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]

  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]

  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]

  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]

  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./left_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = -0.0001
  [../]

  [./right_y]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]

  [./right_z]
    type = DirichletBC
    variable = disp_z
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

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu       '

  line_search = 'none'

  nl_abs_tol = 1e-8

  l_max_its = 20
  dt = 1.0
  num_steps = 1
[]

[Outputs]
  file_base = merged_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
