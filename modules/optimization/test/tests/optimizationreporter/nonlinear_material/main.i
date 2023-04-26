[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'heat_source'
  num_values = '1'
  initial_condition = '0'
  lower_bounds = '0.1'
  upper_bounds = '10000'
  measurement_points = '0.2 0.2 0
                        0.8 0.6 0
                        0.2 1.4 0
                        0.8 1.8 0'
  measurement_values = '1.98404 1.91076 1.56488 1.23863'
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gttol'
  petsc_options_value = ' 1e-5'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [to_forward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/heat_source'
    to_reporters = 'measurement_locations/measurement_xcoord
                    measurement_locations/measurement_ycoord
                    measurement_locations/measurement_zcoord
                    measurement_locations/measurement_time
                    measurement_locations/measurement_values
                    params/heat_source'
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measurement_locations/simulation_values
                      gradient_vpp/inner_product'
    to_reporters = 'OptimizationReporter/simulation_values
                    OptimizationReporter/grad_heat_source'
  []
[]

[Outputs]
  csv = true
[]
