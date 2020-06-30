# This is a mechanical constraint (contact formulation) version of pressurePenalty.i
[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = pressure.e
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = SMALL
    generate_output = 'stress_yy'
  []
[]

[Contact]
  [./m20_s10]
    primary = 20
    secondary = 10
    penalty = 1e8
    formulation = penalty
    tangential_tolerance = 1e-3
    tension_release = -1
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./z]
    type = DirichletBC
    variable = disp_z
    boundary = 5
    value = 0.0
  [../]

  [./Pressure]
    [./press]
      boundary = 7
      factor = 1e3
    [../]
  [../]

  [./down]
    type = DirichletBC
    variable = disp_y
    boundary = 8
    value = -2e-3
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff1_stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'

  line_search = 'none'

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-9

  l_max_its = 100
  nl_max_its = 10
  dt = 1.0
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
