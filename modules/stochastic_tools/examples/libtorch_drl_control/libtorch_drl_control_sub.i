air_density = 1.184 # kg/m3
air_cp = 1000 # J/(kg K)
air_effective_k = 0.5 # W/(m K)

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.0
    xmax = 7.0
    ymin = 0.0
    ymax = 5.0
    nx = 35
    ny = 25
  []
[]

[Variables]
  [T]
    initial_condition = 297
  []
[]

[Kernels]
  [time_derivative]
    type = CoefTimeDerivative
    variable = T
    Coefficient = '${fparse air_density*air_cp}'
  []
  [heat_conduction]
    type = MatDiffusion
    variable = T
    diffusivity = 'k'
  []
[]

[BCs]
  [top_flux]
    type = NeumannBC
    value = 0.0
    boundary = 'top'
    variable = T
  []
  [dirichlet]
    type = FunctionDirichletBC
    function = temp_env
    variable = T
    boundary = 'left right'
  []
[]

[Functions]
  [temp_env]
    type = ParsedFunction
    value = '15.0*sin(t/86400.0*pi) + 273.0'
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
    prop_values = ${air_effective_k}
  []
[]

[Postprocessors]
  [center_temp]
    type = PointValue
    variable = T
    point = '3.5 2.5 0.0'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [center_temp_tend]
    type = PointValue
    variable = T
    point = '3.5 2.5 0.0'
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
  [top_flux]
    type = LibtorchControlValuePostprocessor
    control_name = src_control
  []
  [log_prob_top_flux]
    type = LibtorchDRLLogProbabilityPostprocessor
    control_name = src_control
  []
[]

[Reporters]
  [T_reporter]
    type = AccumulateReporter
    reporters = 'center_temp_tend/value env_temp/value reward/value top_flux/value log_prob_top_flux/value'
  []
[]

[Controls]
  inactive = 'src_control_final'
  [src_control]
    type = LibtorchDRLControl
    parameters = "BCs/top_flux/value"
    responses = 'center_temp_tend env_temp'

    # keep consistent with LibtorchDRLControlTrainer
    input_timesteps = 2
    response_scaling_factors = '0.03 0.03'
    response_shift_factors = '290 290'
    action_standard_deviations = '0.02'
    action_scaling_factors = 200

    execute_on = 'TIMESTEP_BEGIN'
  []
  [src_control_final]
    type = LibtorchNeuralNetControl

    filename = 'mynet_control.net'
    num_neurons_per_layer = '16 6'
    activation_function = 'relu'

    parameters = "BCs/top_flux/value"
    responses = 'center_temp_tend env_temp'

    # keep consistent with LibtorchDRLControlTrainer
    input_timesteps = 2
    response_scaling_factors = '0.03 0.03'
    response_shift_factors = '290 290'
    action_standard_deviations = '0.02'
    action_scaling_factors = 200

    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'

  nl_rel_tol = 1e-7

  start_time = 0.0
  end_time = 86400
  dt = 900.0
[]

[Outputs]
  console = false
  [c]
    type = CSV
    execute_on = FINAL
  []
[]
