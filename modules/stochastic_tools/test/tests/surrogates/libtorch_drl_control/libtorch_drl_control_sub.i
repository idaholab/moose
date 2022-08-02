[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
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

[AuxVariables]
  [reward]
    order = FIRST
  []
[]

[Kernels]
  [time]
    type = ADHeatConductionTimeDerivative
    variable = temp
  []
  [heat_conduc]
    type = ADHeatConduction
    variable = temp
    thermal_conductivity = thermal_conductivity
  []
[]

[AuxKernels]
  [reward_kernel]
    type = FunctionAux
    variable = reward
    function = reward_fcn
  []
[]

[BCs]
  [top_flux]
    type = ADNeumannBC
    value = 1e3 # W/m^2
    boundary = 'left'
    variable = temp
  []
  [dirichlet]
    type = ADFunctionDirichletBC
    function = temp_env
    variable = temp
    boundary = 'right'
  []
[]

[Functions]
  [specific_heat_air]
    type = ConstantFunction
    value = 1.006301824 # kJ/kgK
  []
  [thermal_conductivity_air]
    type = ConstantFunction
    value = 26.53832364 # mW/mK
  []
  [temp_env]
    type = ParsedFunction
    value = '15.0*sin(t/86400.0 *pi) + 273.0'
  []
  [design_fcn]
    type = ParsedFunction
    value = '297'
  []
  [reward_fcn]
    type = RewardFunction
    design_function = design_fcn
    observed_value = center_temp_tend
    c1 = 1000
    c2 = 1
    # execute_on = 'TIMESTEP_END'
  []
[]

[Materials]
  [heat]
    type = ADHeatConductionMaterial
    specific_heat_temperature_function = specific_heat_air
    thermal_conductivity_temperature_function = thermal_conductivity_air
    temp = temp
  []
  [density]
    type = ADDensity
    density = 1.225 # kg/m^3
  []
  [T_env]
    type = ADCoupledValueFunctionMaterial
    function = temp_env
    prop_name = T_env
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
  petsc_options_value = 'preonly lu       superlu_dist NONZERO 1e-10'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  end_time = 86400.0
  dt = 1800.0
  dtmin = 1e-4

  #num_steps = 2

  # auto_advance = true # cut time-step when subapp fails
  #
  # error_on_dtmin = false
[]

[Outputs]
  # file_base = output/heat_conduction_out
  csv = false
  exodus = false
  console = false
  execute_on = 'FINAL'
[]

[Postprocessors]
  # temperature observations
  [center_temp]
    type = PointValue
    variable = temp
    point = '3.5 2.5 1.75'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # time end response (should be executed on LINEAR to avoid dependency issues)
  [center_temp_tend]
    type = PointValue
    variable = temp
    point = '3.5 2.5 1.75'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [env_temp]
    type = FunctionValuePostprocessor
    function = temp_env
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # [top1_temp]
  #   type = PointValue
  #   variable = temp
  #   point = '3.5 2.5 3.5'
  #   execute_on = 'TIMESTEP_BEGIN'
  # []
  # [top2_temp]
  #   type = PointValue
  #   variable = temp
  #   point = '5.5 4.0 3.5'
  #   execute_on = 'TIMESTEP_BEGIN'
  # []
  # [bottom_temp]
  #   type = PointValue
  #   variable = temp
  #   point = '3.5 2.5 1'
  #   execute_on = 'TIMESTEP_BEGIN'
  # []
  # [left_temp]
  #   type = PointValue
  #   variable = temp
  #   point = '2 2.5 1.75'
  #   execute_on = 'TIMESTEP_BEGIN'
  # []
  # reward (should be executed on TIMESTEP_END)
  [reward]
    type = FunctionValuePostprocessor
    function = reward_fcn
    execute_on = 'INITIAL TIMESTEP_END'
    indirect_dependencies = 'center_temp_tend'
  []
  # received control signal
  [top_flux]
    type = Receiver
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [log_prob_top_flux]
    type = Receiver
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Reporters]
  [T_reporter]
    type = AccumulateReporter
    reporters = 'center_temp/value env_temp/value reward/value top_flux/value '
                'log_prob_top_flux/value'
    # ' top1_temp/value top2_temp/value bottom_temp/value left_temp/value'
  []
[]

[Controls]
  [src_control]
    type = LibtorchNNControl
    parameters = "BCs/top_flux/value"
    action_postprocessors = "top_flux"
    log_probability_postprocessors = "log_prob_top_flux"
    responses = 'center_temp env_temp' #' top1_temp top2_temp bottom_temp left_temp '

    # keep consistent with LibtorchDRLControlTrainer
    input_timesteps = 1
    input_scale_factor = 0.03
    input_shift_factor = -270
    output_scale_factor = 200
    standard_deviation_value = 1

    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Outputs]
  [c]
    type = CSV
    execute_on = FINAL
  []
[]
