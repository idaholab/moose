# This tests that a linear and constant function can be scaled in
# two separate functionNeumannBCs both applied to the same sideset using
# two parsed functions.  The scale of the linear and constant functions
# are being parameterized.

[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'left'
  num_values = '2'
  initial_condition = '10 10'
  lower_bounds = '0'
  upper_bounds = '1000'

  inequality_names = 'ineq'

  measurement_file = 'measurementData.csv'
  file_xcoord = 'coordx'
  file_ycoord ='y'
  file_zcoord = 'z'
  file_value = 'measured_value'
[]

[Executioner]
  type = Optimize
  tao_solver = taoalmm
  petsc_options_iname = '-tao_gatol -tao_catol  -tao_almm_type -tao_almm_mu_factor -tao_almm_mu_init'
  petsc_options_value = ' 1e-1 1e-1  phr  1.1 1.0'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = "FORWARD"
  []
[]
[Reporters]
  [optInfo]
    type = OptimizationInfo
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
                      OptimizationReporter/left'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/left'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/simulation_values
                      grad_bc_left/inner_product
                      ineq/ineq
                      gradient_c/gradient_c'
    to_reporters = 'OptimizationReporter/simulation_values
                    OptimizationReporter/grad_left
                    OptimizationReporter/ineq
                    OptimizationReporter/grad_ineq'
  []
[]

