
[Mesh]
  file = incremental_slip.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
  [../]
[]

[Functions]
  [./secondary_x]
    type = PiecewiseLinear
    x = '0 1 2   3 4 5  6    7 8   9'
    y = '0 0 0.5 0 0 0 -0.25 0 0.5 0'
  [../]
  [./secondary_y]
    type = PiecewiseLinear
    x = '0  1     9'
    y = '0 -0.15 -0.15'
  [../]
  [./secondary_z]
    type = PiecewiseLinear
    x = '0 1  2   3 4 5 6    7  8   9'
    y = '0 0 -0.5 0 0 0 0.25 0 -0.5 0'
  [../]

  [./primary_x]
    type = PiecewiseLinear
    x = '0 1  2 3 4   5 6    7 8   9'
    y = '0 0  0 0 0.5 0 0.25 0 0.5 0'
  [../]
  [./primary_y]
    type = PiecewiseLinear
    x = '0 9'
    y = '0 0'
  [../]
  [./primary_z]
    type = PiecewiseLinear
    x = '0 1  2 3 4   5  6    7  8   9'
    y = '0 0  0 0 0.5 0 -0.25 0 -0.5 0'
  [../]
[]

[AuxVariables]
  [./inc_slip_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./inc_slip_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./inc_slip_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./inc_slip_x]
    type = PenetrationAux
    variable = inc_slip_x
    quantity = incremental_slip_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./inc_slip_y]
    type = PenetrationAux
    variable = inc_slip_y
    quantity = incremental_slip_y
    boundary = 3
    paired_boundary = 2
  [../]
  [./inc_slip_z]
    type = PenetrationAux
    variable = inc_slip_z
    quantity = incremental_slip_z
    boundary = 3
    paired_boundary = 2
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 2
    secondary = 3
    penalty = 1e7
  [../]
[]

[BCs]
  [./secondary_x]
    type = FunctionDirichletBC
    variable = disp_x
    preset = false
    boundary = 4
    function = secondary_x
  [../]
  [./secondary_y]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = 4
    function = secondary_y
  [../]
  [./secondary_z]
    type = FunctionDirichletBC
    variable = disp_z
    preset = false
    boundary = 4
    function = secondary_z
  [../]

  [./primary_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '1 2'
    function = primary_x
  [../]
  [./primary_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '1 2'
    function = primary_y
  [../]
  [./primary_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = '1 2'
    function = primary_z
  [../]
[] # BCs

[Materials]
  [./elasticity_tensor_1]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 1.0e6
    poissons_ratio = 0.0
  [../]
  [./stress_1]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]

  [./elasticity_tensor_2]
    type = ComputeIsotropicElasticityTensor
    block = 2
    youngs_modulus = 1.0e6
    poissons_ratio = 0.0
  [../]
  [./stress_2]
    type = ComputeFiniteStrainElasticStress
    block = 2
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu            superlu_dist'

  line_search = 'none'

  nl_abs_tol = 1e-8
  l_max_its = 100
  nl_max_its = 10
  dt = 1.0
  num_steps = 9
[] # Executioner

[Outputs]
  exodus = true
[]
