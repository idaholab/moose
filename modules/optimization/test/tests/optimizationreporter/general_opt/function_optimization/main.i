[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  parameter_names = 'vals'
  num_values = '2'
  objective_name = obj_value
[]

[Problem]
  solve = false
[]
[Executioner]
  type = Optimize
  tao_solver = taobqnktr
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-8 '
  verbose = true
  output_optimization_iterations = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/vals'
    to_reporters = 'vals/vals'
  []

  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'obj_pp/value
                      grad_f/grad_f'
    to_reporters = 'OptimizationReporter/obj_value
                    OptimizationReporter/grad_vals'
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_system_information_on = none
  []
  [json_forward]
    type = JSON
    execute_on = 'FORWARD '
    execute_system_information_on = none
  []
[]
