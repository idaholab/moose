[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = obj_value
  parameter_names = 'E sigma_y C eta'
  num_values = '1 1 1 1'
  initial_condition = '1; 1; 1; 1'
  lower_bounds = '0.1; 0.1; 0.1; 0.1'
  upper_bounds = '10; 10; 10; 10'
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  petsc_options_iname = '-tao_gatol -tao_max_it'
  petsc_options_value = '1e-12 50'
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint_vjp.i
    execute_on = 'FORWARD'
  []
[]

[Transfers]
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/E
                      OptimizationReporter/sigma_y
                      OptimizationReporter/C
                      OptimizationReporter/eta'
    to_reporters = 'params/E
                    params/sigma_y
                    params/C
                    params/eta'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/objective_value
                      grad_E/inner_product
                      grad_sigma_y/inner_product
                      grad_C/inner_product
                      grad_eta/inner_product'
    to_reporters = 'OptimizationReporter/obj_value
                    OptimizationReporter/grad_E
                    OptimizationReporter/grad_sigma_y
                    OptimizationReporter/grad_C
                    OptimizationReporter/grad_eta'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
    items = 'current_iterate function_value gnorm'
  []
[]

[Outputs]
  console = false
  csv = true
[]
