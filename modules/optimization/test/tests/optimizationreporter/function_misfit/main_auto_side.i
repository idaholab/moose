measurement_points = '0.2 1.4 0
   0.9 1.4 0'
measurement_values = '289 302.3'

[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'parameter_results'
  num_values = '3'
[]

[Reporters]
  [main]
    type = OptimizationData
    measurement_points = ${measurement_points}
    measurement_values = ${measurement_values}
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnktr
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-8'
  ## THESE OPTIONS ARE FOR TESTING THE ADJOINT GRADIENT
  # petsc_options_iname = '-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value = '1 true true false 1e-3 1e10'
  # petsc_options = '-tao_test_gradient_view'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint_side.i
    execute_on = "FORWARD"
  []
[]

[Transfers]
  # FORWARD transfers
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/measurement_values
                      OptimizationReporter/parameter_results'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    point_source/value'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'objective/value
                      gradient/temperature_adjoint'
    to_reporters = 'OptimizationReporter/objective_value
                    OptimizationReporter/grad_parameter_results'
  []
[]

[Outputs]
  csv = true
[]
