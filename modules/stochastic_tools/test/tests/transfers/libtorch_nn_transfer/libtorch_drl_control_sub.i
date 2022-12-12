[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.0
    xmax = 7.0
    nx = 3
  []
[]

[Variables]
  [temp]
    initial_condition = 300
  []
[]

[Kernels]
  [time]
    type = CoefTimeDerivative
    variable = temp
    Coefficient = '${fparse 1.00630182*1.225}'
  []
  [heat_conduc]
    type = MatDiffusion
    variable = temp
    diffusivity = 'k'
  []
[]

[BCs]
  [left_flux]
    type = NeumannBC
    value = 0.0
    boundary = 'left'
    variable = temp
  []
  [dirichlet]
    type = FunctionDirichletBC
    function = temp_env
    variable = temp
    boundary = 'right'
  []
[]

[Functions]
  [temp_env]
    type = ParsedFunction
    value = '15.0*sin(t/86400.0 *pi) + 273.0'
  []
  [design_function]
    type = ParsedFunction
    value = '297'
  []
  [reward_function]
    type = ScaledAbsDifferenceDRLRewardFunction
    design_function = design_function
    observed_value = center_temp_tend
    c1 = 1
    c2 = 10
  []
[]

[Materials]
  [constant]
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = 26.53832364
  []
[]

[Postprocessors]
  [center_temp]
    type = PointValue
    variable = temp
    point = '3.5 0.0 0.0'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [center_temp_tend]
    type = PointValue
    variable = temp
    point = '3.5 0.0 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [env_temp]
    type = FunctionValuePostprocessor
    function = temp_env
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [reward]
    type = FunctionValuePostprocessor
    function = reward_function
    execute_on = 'INITIAL TIMESTEP_END'
    indirect_dependencies = 'center_temp_tend env_temp'
  []
  [left_flux]
    type = LibtorchControlValuePostprocessor
    control_name = src_control
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [log_prob_left_flux]
    type = LibtorchDRLLogProbabilityPostprocessor
    control_name = src_control
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Reporters]
  [T_reporter]
    type = AccumulateReporter
    reporters = 'center_temp_tend/value env_temp/value reward/value left_flux/value log_prob_left_flux/value'
    outputs = 'csv_out'
  []
  [nn_parameters]
    type = LibtorchArtificialNeuralNetParameters
    control_name = src_control
    outputs = json_out
  []
[]

[Controls]
  [src_control]
    type = LibtorchDRLControl
    parameters = "BCs/left_flux/value"
    responses = 'center_temp env_temp'

    # keep consistent with LibtorchDRLControlTrainer
    input_timesteps = 2
    response_scaling_factors = '0.03 0.03'
    response_shift_factors = '270 270'
    action_standard_deviations = '0.1'
    action_scaling_factors = 100

    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'

  nl_rel_tol = 1e-8

  start_time = 0.0
  end_time = 86400
  dt = 14400.0
[]

[Outputs]
  [json_out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
[]
