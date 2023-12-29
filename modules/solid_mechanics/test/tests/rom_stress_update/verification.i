[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [./temperature]
  [../]
[]

[AuxKernels]
  [./temp_aux]
    type = FunctionAux
    variable = temperature
    function = temp_fcn
    execute_on = 'initial timestep_begin'
  [../]
[]

[Functions]
  [./rhom_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    xy_in_file_only = false
    direction = right
  [../]
  [./rhoi_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 2
    format = columns
    xy_in_file_only = false
    direction = right
  [../]
  [./vmJ2_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 3
    format = columns
    xy_in_file_only = false
    direction = right
  [../]
  [./evm_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 4
    format = columns
    xy_in_file_only = false
    direction = right
  [../]
  [./temp_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 5
    format = columns
    xy_in_file_only = false
    direction = right
  [../]

  [./rhom_soln_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 7
    format = columns
    xy_in_file_only = false
    direction = right
  [../]
  [./rhoi_soln_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 8
    format = columns
    xy_in_file_only = false
    direction = right
  [../]
  [./creep_rate_soln_fcn]
    type = PiecewiseConstant
    data_file = ss316_verification_data.csv
    x_index_in_file = 0
    y_index_in_file = 10
    format = columns
    xy_in_file_only = false
    direction = right
  [../]

  [./rhom_diff_fcn]
    type = ParsedFunction
    symbol_names = 'rhom_soln rhom'
    symbol_values = 'rhom_soln rhom'
    expression = 'abs(rhom_soln - rhom) / rhom_soln'
  [../]
  [./rhoi_diff_fcn]
    type = ParsedFunction
    symbol_names = 'rhoi_soln rhoi'
    symbol_values = 'rhoi_soln rhoi'
    expression = 'abs(rhoi_soln - rhoi) / rhoi_soln'
  [../]
  [./creep_rate_diff_fcn]
    type = ParsedFunction
    symbol_names = 'creep_rate_soln creep_rate'
    symbol_values = 'creep_rate_soln creep_rate'
    expression = 'abs(creep_rate_soln - creep_rate) / creep_rate_soln'
  [../]
[]


[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'vonmises_stress'
  [../]
[]

[BCs]
  [./symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./pressure_x]
    type = Pressure
    variable = disp_x
    boundary = right
    function = vmJ2_fcn
    factor = 0.5e6
  [../]
  [./pressure_y]
    type = Pressure
    variable = disp_y
    boundary = top
    function = vmJ2_fcn
    factor = -0.5e6
  [../]
  [./pressure_z]
    type = Pressure
    variable = disp_z
    boundary = front
    function = vmJ2_fcn
    factor = -0.5e6
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e11
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction
  [../]
  [./rom_stress_prediction]
    type = SS316HLAROMANCEStressUpdateTest
    temperature = temperature
    effective_inelastic_strain_name = effective_creep_strain
    internal_solve_full_iteration_history = true
    outputs = all
    wall_dislocation_density_forcing_function = rhoi_fcn
    cell_dislocation_density_forcing_function = rhom_fcn
    old_creep_strain_forcing_function = evm_fcn
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  petsc_options = '-snes_ksp_ew -snes_converged_reason -ksp_converged_reason'# -ksp_error_if_not_converged -snes_error_if_not_converged'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  automatic_scaling = true
  compute_scaling_once = false

  nl_abs_tol = 1e-10

  dt = 1e-3
  end_time = 1e-2
[]

[Postprocessors]
  [./effective_strain_avg]
    type = ElementAverageValue
    variable = effective_creep_strain
    outputs = console
  [../]
  [./temperature]
    type = ElementAverageValue
    variable = temperature
    outputs = console
  [../]
  [./rhom]
    type = ElementAverageValue
    variable = cell_dislocations
  [../]
  [./rhoi]
    type = ElementAverageValue
    variable = wall_dislocations
  [../]
  [./vonmises_stress]
    type = ElementAverageValue
    variable = vonmises_stress
    outputs = console
  [../]
  [./creep_rate]
    type = ElementAverageValue
    variable = creep_rate
  [../]
  [./rhom_in]
    type = FunctionValuePostprocessor
    function = rhom_fcn
    execute_on = 'TIMESTEP_END initial'
    outputs = console
  [../]
  [./rhoi_in]
    type = FunctionValuePostprocessor
    function = rhoi_fcn
    execute_on = 'TIMESTEP_END initial'
    outputs = console
  [../]
  [./vmJ2_in]
    type = FunctionValuePostprocessor
    function = vmJ2_fcn
    execute_on = 'TIMESTEP_END initial'
    outputs = console
  [../]
  [./rhom_soln]
    type = FunctionValuePostprocessor
    function = rhom_soln_fcn
    outputs = console
  [../]
  [./rhoi_soln]
    type = FunctionValuePostprocessor
    function = rhoi_soln_fcn
    outputs = console
  [../]
  [./creep_rate_soln]
    type = FunctionValuePostprocessor
    function = creep_rate_soln_fcn
    outputs = console
  [../]

  [./rhom_diff]
    type = FunctionValuePostprocessor
    function = rhom_diff_fcn
    outputs = console
  [../]
  [./rhoi_diff]
    type = FunctionValuePostprocessor
    function = rhoi_diff_fcn
    outputs = console
  [../]
  [./creep_rate_diff]
    type = FunctionValuePostprocessor
    function = creep_rate_diff_fcn
    outputs = console
  [../]

  [./rhom_max_diff]
    type = TimeExtremeValue
    postprocessor = rhom_diff
    outputs = console
  [../]
  [./rhoi_max_diff]
    type = TimeExtremeValue
    postprocessor = rhoi_diff
    outputs = console
  [../]
  [./creep_rate_max_diff]
    type = TimeExtremeValue
    postprocessor = creep_rate_diff
    outputs = console
  [../]
[]

[Outputs]
  csv = true
  file_base = 'verification_1e-3_out'
[]
