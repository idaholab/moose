[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.0
    xmax = 7.0
    nx = 20
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

[Controls]
  inactive = src_control_empty
  [src_control]
    seed = 11
    type = LibtorchDRLControl
    parameters = "BCs/left_flux/value"
    responses = 'center_temp env_temp'

    input_timesteps = 2
    response_scaling_factors = '0.03 0.03'
    response_shift_factors = '270 270'
    action_standard_deviations = '0.1'
    action_scaling_factors = 200

    filename = 'mynet_control.net'
    torch_script_format = false
    num_neurons_per_layer = '16 6'
    activation_function = 'relu'

    execute_on = 'TIMESTEP_BEGIN'
  []
  [src_control_empty]
    type = LibtorchDRLControl
    parameters = "BCs/left_flux/value"
    responses = 'center_temp env_temp'

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
  end_time = 18000
  dt = 1800.0
[]

[Outputs]
  csv = true
[]
