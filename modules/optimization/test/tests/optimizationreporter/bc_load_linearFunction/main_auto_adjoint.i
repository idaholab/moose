# This tests that a linear and constant function can be scaled in
# two separate functionNeumannBCs both applied to the same sideset using
# two parsed functions.  The scale of the linear and constant functions
# are being parameterized.

[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'left right'
  num_values = '2 1'
  measurement_file = 'measurementData.csv'
  file_xcoord = 'coordx'
  file_ycoord ='y'
  file_zcoord = 'z'
  file_value = 'measured_value'
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  petsc_options_iname = '-tao_gatol -tao_ls_type'
  petsc_options_value = '1e-5 unit'
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
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/left
                      OptimizationReporter/right'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/left
                    params/right'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/simulation_values
                      grad_bc_left/inner_product
                      grad_bc_right/inner_product'
    to_reporters = 'OptimizationReporter/simulation_values
                    OptimizationReporter/grad_left
                    OptimizationReporter/grad_right'
  []
[]
