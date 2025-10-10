# Total variation regularization test
# Demonstrates edge-preserving parameter recovery for step function source
# OptimizationReporter is in a seperate input file

[Optimization]
[]

[Reporters]
  [measurement_data]
    type = OptimizationData
    measurement_file = 'synthetic_data_out_point_data_0002.csv'
    file_value = 'temperature'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  petsc_options_iname = '-tao_max_it -tao_gatol '
  petsc_options_value = '250 1e-4'
  verbose = true
[]

[MultiApps]
  [forward_and_adjoint]
    type = FullSolveMultiApp
    input_files = 'forward_and_adjoint.i'
    execute_on = 'FORWARD'
  []
[]

[Transfers]
  [to_forward]
    type = MultiAppReporterTransfer
    to_multi_app = forward_and_adjoint
    from_reporters = 'measurement_data/measurement_xcoord
                      measurement_data/measurement_ycoord
                      measurement_data/measurement_zcoord
                      measurement_data/measurement_time
                      measurement_data/measurement_values
                      OptimizationReporter/source'
    to_reporters = 'measurement_data/measurement_xcoord
                    measurement_data/measurement_ycoord
                    measurement_data/measurement_zcoord
                    measurement_data/measurement_time
                    measurement_data/measurement_values
                    parameters/source'
  []
  [from_forward_and_adjoint]
    type = MultiAppReporterTransfer
    from_multi_app = forward_and_adjoint
    from_reporters = 'measurement_data/objective_value
                      gradient/inner_product'
    to_reporters = 'OptimizationReporter/objective_value
                    OptimizationReporter/grad_source'
  []
[]

[Outputs]
  console = true
[]
