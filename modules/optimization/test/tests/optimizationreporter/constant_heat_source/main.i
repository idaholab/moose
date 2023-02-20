[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'parameter_results'
  num_values = '1'
  initial_condition = '500'
  lower_bounds = '0.1'
  upper_bounds = '10000'
  measurement_points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measurement_values = '226 254 214 146'
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-6'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = ADJOINT
  []
[]

[Transfers]
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
                    params/q'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/misfit_values
                      OptimizationReporter/parameter_results'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    params/q'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradient_vpp/inner_product'
    to_reporters = 'OptimizationReporter/grad_parameter_results'
  []
[]

[Outputs]
  csv = true
[]
