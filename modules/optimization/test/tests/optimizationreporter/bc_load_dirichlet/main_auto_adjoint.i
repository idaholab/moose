[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'left'
  num_values = '2'
  initial_condition = '100 0' # true values are '200 25'
[]

[Reporters]
  [main]
    type = OptimizationData
    measurement_file = 'measurementData.csv'
  []
  [optInfo]
    type = OptimizationInfo
    items = 'current_iterate function_value gnorm'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  petsc_options_iname = '-tao_gatol -tao_ls_type'
  petsc_options_value = '1e-4 unit'
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
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/measurement_values
                      OptimizationReporter/left'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/vals'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/objective_value
                      grad_bc_left/inner_product'
    to_reporters = 'OptimizationReporter/objective_value
                    OptimizationReporter/grad_left'
  []
[]

[Outputs]
  csv = true
  console = false
[]
