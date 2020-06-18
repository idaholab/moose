[GlobalParams]
  volumetric_locking_correction= false
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = merged.e
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
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
  [./stiffStuff]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stiffStuff_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
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
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
