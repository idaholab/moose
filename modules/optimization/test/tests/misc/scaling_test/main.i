[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'parameter_results'
  num_values = '1'
  initial_condition = '1.05'
  lower_bounds = '0.001'
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnktr
  # These options are to force an initial residual evaluation only.
  petsc_options_iname = '-tao_max_it -tao_gatol'
  petsc_options_value = '1  1e100'
  verbose = true
  output_optimization_iterations = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = "FORWARD"
  []
[]

[Transfers]
  # FORWARD transfers
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'params/k'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'obj/value
                     gradient/inner'
    to_reporters = 'OptimizationReporter/objective_value
                    OptimizationReporter/grad_parameter_results'
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = 'FORWARD'
    execute_system_information_on = NONE
  []
[]
