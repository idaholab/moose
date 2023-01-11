[Mesh]
  file = blocks_2d_nogap.e
[]

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./penetration]
  [../]
  [./inc_slip_x]
  [../]
  [./inc_slip_y]
  [../]
  [./accum_slip_x]
  [../]
  [./accum_slip_y]
  [../]
[]

[Functions]
  [./vertical_movement]
    type = ParsedFunction
    expression = -t
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
  [../]
[]

[AuxKernels]
  [./zeroslip_x]
    type = ConstantAux
    variable = inc_slip_x
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
  [../]
  [./zeroslip_y]
    type = ConstantAux
    variable = inc_slip_y
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
  [../]
  [./accum_slip_x]
    type = AccumulateAux
    variable = accum_slip_x
    accumulate_from_variable = inc_slip_x
    execute_on = timestep_end
  [../]
  [./accum_slip_y]
    type = AccumulateAux
    variable = accum_slip_y
    accumulate_from_variable = inc_slip_y
    execute_on = timestep_end
  [../]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
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
  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    #Initial gap is 0.01
    value = -0.01
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = vertical_movement
  [../]
[]

[Materials]
  [./left]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e7
    poissons_ratio = 0.3
  [../]
  [./right]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 1000
  dt = 0.01
  end_time = 0.10
  num_steps = 1000
  l_tol = 1e-6
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  dtmin = 0.01

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Contact]
  [./leftright]
    primary = 2
    secondary = 3
    model = glued
    formulation = penalty
    penalty = 1e+7
  [../]
[]
