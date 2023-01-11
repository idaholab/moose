#

[Mesh]
  file = cracking_test.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./displx]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0.00175'
  [../]
  [./velocity_y]
    type = ParsedFunction
    expression = 'if(t < 2, 0.00175, 0)'
  [../]
  [./velocity_z]
    type = ParsedFunction
    expression = 0.00175
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  [../]
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./move_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = displx
  [../]

  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./move_y]
    type = PresetVelocity
    variable = disp_y
    boundary = 5
    function = velocity_y
#    time_periods = 'p2 p3'
  [../]

  [./fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 3
    value = 0.0
  [../]
  [./move_z]
    type = PresetVelocity
    variable = disp_z
    boundary = 6
    function = velocity_z
#    time_periods = 'p3'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 186.5e9
    poissons_ratio = .316
  [../]
  [./elastic_stress]
    type = ComputeSmearedCrackingStress
    cracking_stress = 119.3e6
    softening_models = exponential_softening
  [../]
  [./exponential_softening]
    type = ExponentialSoftening
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '101                lu'


  line_search = 'none'


  l_max_its = 100
  l_tol = 1e-5

  nl_max_its = 100
  nl_abs_tol = 1e-4
  #nl_rel_tol = 1e-4
  nl_rel_tol = 1e-6

  start_time = 0.0
  end_time = 3.0
  dt = 0.01
[]

[Controls]
  [./p1]
    type = TimePeriod
    start_time = 0.0
    end_time = 1.0
    disable_objects = 'BCs/move_y BCs/move_z'
    reverse_on_false = false
    execute_on = 'initial timestep_begin'
  [../]

  [./p2]
    type = TimePeriod
    start_time = 1.0
    end_time = 2.0
    disable_objects = 'BCs/move_z'
    enable_objects = 'BCs/move_y'
    reverse_on_false = false
    execute_on = 'initial timestep_begin'
  [../]

  [./p3]
    type = TimePeriod
    start_time = 2.0
    end_time = 3.0
    enable_objects = 'BCs/move_y BCs/move_z'
    reverse_on_false = false
    execute_on = 'initial timestep_begin'
    set_sync_times = true
  [../]
[]

[Outputs]
  exodus = true
[]
