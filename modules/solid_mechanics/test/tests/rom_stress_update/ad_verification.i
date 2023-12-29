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
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [rhoi_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 2
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [vmJ2_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 3
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [evm_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 4
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [temp_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 5
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []

  [rhom_soln_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 7
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [rhoi_soln_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 8
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
  [creep_rate_soln_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 10
    format = columns
    xy_in_file_only = false
    direction = LEFT_INCLUSIVE
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'vonmises_stress'
    use_automatic_differentiation = true
  []
[]

[BCs]
  [symmx]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmy]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmz]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [pull_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = right
    value = 1e-5 # This is required to make a non-zero effective trial stress so radial return is engaged
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    shear_modulus = 1e13
    poissons_ratio = 0.3
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction
  []
  [rom_stress_prediction]
    type = ADSS316HLAROMANCEStressUpdateTest
    temperature = temperature
    effective_inelastic_strain_name = effective_creep_strain
    internal_solve_full_iteration_history = true
    apply_strain = false
    outputs = all
    wall_dislocation_density_forcing_function = rhoi_fcn
    cell_dislocation_density_forcing_function = rhom_fcn
    old_creep_strain_forcing_function = evm_fcn
    wall_input_window_low_failure = ERROR
    wall_input_window_high_failure = ERROR
    cell_input_window_low_failure = ERROR
    cell_input_window_high_failure = ERROR
    temperature_input_window_low_failure = ERROR
    temperature_input_window_high_failure = ERROR
    stress_input_window_low_failure = ERROR
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

  dt = 1e-3
  end_time = 1e-2
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
    outputs = console
  []
  [rhom]
    type = ElementAverageValue
    variable = cell_dislocations
  []
  [rhoi]
    type = ElementAverageValue
    variable = wall_dislocations
  []
  [creep_rate]
    type = ElementAverageValue
    variable = creep_rate
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
    outputs = console
  []
  [rhom_soln]
    type = FunctionValuePostprocessor
    function = rhom_soln_fcn
    outputs = console
  []
  [rhoi_soln]
    type = FunctionValuePostprocessor
    function = rhoi_soln_fcn
    outputs = console
  []
  [creep_rate_soln]
    type = FunctionValuePostprocessor
    function = creep_rate_soln_fcn
  []

  [rhom_diff]
    type = ParsedPostprocessor
    pp_names = 'rhom_soln rhom'
    function = '(rhom_soln - rhom) / rhom_soln'
    outputs = console
  []
  [rhoi_diff]
    type = ParsedPostprocessor
    pp_names = 'rhoi_soln rhoi'
    function = '(rhoi_soln - rhoi) / rhoi_soln'
    outputs = console
  []
  [creep_rate_diff]
    type = ParsedPostprocessor
    pp_names = 'creep_rate creep_rate_soln'
    function = '(creep_rate_soln - creep_rate) / creep_rate_soln'
    outputs = console
  []

  [z_rhom_max_diff]
    type = TimeExtremeValue
    postprocessor = rhom_diff
    value_type = abs_max
  []
  [z_rhoi_max_diff]
    type = TimeExtremeValue
    postprocessor = rhoi_diff
    value_type = abs_max
  []
  [z_creep_rate_max_diff]
    type = TimeExtremeValue
    postprocessor = creep_rate_diff
    value_type = abs_max
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL TIMESTEP_END FINAL'
[]
