# Tests the tile and partition assembly for overlapping partitions and
# a variety of different overlapping tile conditions.
# Creep_rate should always be 2.718281828459

endtime = 1.9

[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [temperature]
  []
[]

[AuxKernels]
  [temp_aux]
    type = FunctionAux
    variable = temperature
    function = temp_fcn
    execute_on = 'initial timestep_begin'
  []
[]

[Functions]
  [rhom_fcn]
    type = PiecewiseConstant
    x = '0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9'
    y = '5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12 5.7e12'
    direction = LEFT_INCLUSIVE
  []
  [rhoi_fcn]
    type = PiecewiseConstant
    x = '0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9'
    y = '4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11 4.83e11'
    direction = LEFT_INCLUSIVE
  []
  [vmJ2_fcn]
    type = PiecewiseConstant
    x = '0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9'
    y = '25.68 25.68 45.0 55.28 63.0 67.12 85.0 85.0 85.0 85.0 85.0 85.0 55.28 63.0 67.12 63.0 63.0 55.28 96.72 63.0'
    direction = LEFT_INCLUSIVE
  []
  [evm_fcn]
    type = PiecewiseConstant
    x = '0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9'
    y = '0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01 0.01'
    direction = LEFT_INCLUSIVE
  []
  [temp_fcn]
    type = PiecewiseConstant
    x = '0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9'
    y = '940.0 940.0 940.0 940.0 940.0 940.0 940.0 905.0 897.0 881.0 860.0 821.0 860.0 881.0 897.0 897.0 905.0 897.0 860.0 860.0'
    direction = LEFT_INCLUSIVE
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'vonmises_stress'
  []
[]

[BCs]
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [pull_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 1e-5 # This is required to make a non-zero effective trial stress so radial return is engaged
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    shear_modulus = 1e13
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction
  []
  [rom_stress_prediction]
    type = LAROMANCE3TileTest
    temperature = temperature
    effective_inelastic_strain_name = effective_creep_strain
    internal_solve_full_iteration_history = true
    apply_strain = false
    outputs = all
    verbose = true
    wall_dislocation_density_forcing_function = rhoi_fcn
    cell_dislocation_density_forcing_function = rhom_fcn
    old_creep_strain_forcing_function = evm_fcn
    wall_input_window_low_failure = ERROR
    wall_input_window_high_failure = ERROR
    cell_input_window_low_failure = ERROR
    cell_input_window_high_failure = ERROR
    temperature_input_window_low_failure = DONOTHING
    temperature_input_window_high_failure = ERROR
    stress_input_window_low_failure = DONOTHING
    stress_input_window_high_failure = ERROR
    old_strain_input_window_low_failure = ERROR
    old_strain_input_window_high_failure = ERROR
    environment_input_window_low_failure = ERROR
    environment_input_window_high_failure = ERROR
    effective_stress_forcing_function = vmJ2_fcn
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-1 # Nothing is really being solved here, so loose tolerances are okay

  dt = 0.1
  end_time = ${endtime}
  timestep_tolerance = 1e-3
[]

[Postprocessors]
  [extrapolation]
    type = ElementAverageValue
    variable = ROM_extrapolation
    outputs = console
  []
  [old_strain_in]
    type = FunctionValuePostprocessor
    function = evm_fcn
    execute_on = 'TIMESTEP_END initial'
    outputs = console
  []
  [temperature]
    type = ElementAverageValue
    variable = temperature
  []
  [partition_weight]
    type = ElementAverageMaterialProperty
    mat_prop = partition_weight
  []
  [rhom_in]
    type = FunctionValuePostprocessor
    function = rhom_fcn
    execute_on = 'TIMESTEP_END initial'
    outputs = console
  []
  [rhoi_in]
    type = FunctionValuePostprocessor
    function = rhoi_fcn
    execute_on = 'TIMESTEP_END initial'
    outputs = console
  []
  [vmJ2_in]
    type = FunctionValuePostprocessor
    function = vmJ2_fcn
    execute_on = 'TIMESTEP_END initial'
  []
  [creep_rate]
    type = ElementAverageMaterialProperty
    mat_prop = creep_rate
  []
  [rhom_rate]
    type = ElementAverageMaterialProperty
    mat_prop = cell_dislocation_rate
  []
  [rhoi_rate]
    type = ElementAverageMaterialProperty
    mat_prop = wall_dislocation_rate
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL TIMESTEP_END FINAL'
[]
