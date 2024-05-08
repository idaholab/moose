# This tests constrained optimization of a linear and constant function
# that are used to apply NuemannBCs on a side.

[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'left'
  num_values = '2'
  initial_condition = '10 10'
  lower_bounds = '0'
  upper_bounds = '1000'

  inequality_names = 'ineq'

[]
[Reporters]
  [main]
    type = OptimizationData
    measurement_points = '0.2 0.2 0'
    measurement_values = '207'
    file_value = 'measured_value'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taoalmm
  petsc_options_iname = '-tao_gatol -tao_catol  -tao_almm_type -tao_almm_mu_factor -tao_almm_mu_init  -tao_almm_subsolver_tao_type'
  petsc_options_value = ' 1e-3 1e-3  phr  1.1 1.0 bqnktr'
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
                    params/left'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/objective_value
                      grad_bc_left/inner_product
                      ineq/ineq
                      gradient_c/gradient_c'
    to_reporters = 'OptimizationReporter/objective_value
                    OptimizationReporter/grad_left
                    OptimizationReporter/ineq
                    OptimizationReporter/grad_ineq'
  []
[]

