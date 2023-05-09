[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'parameter_results'
  num_values = '3'
  measurement_points = '0.5 0.28 0
                        0.5 0.6 0
                        0.5 0.8 0
                        0.5 1.1 0'
  measurement_values = '293 304 315 320'
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname='-tao_gttol -tao_ls_type'
  petsc_options_value='1e-5 unit'
  verbose = true
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
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
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
    from_reporters = 'measure_data/simulation_values
                      gradient/temperature_adjoint'
    to_reporters = 'OptimizationReporter/simulation_values
                    OptimizationReporter/grad_parameter_results'
  []
[]

[Outputs]
  csv = true
[]
