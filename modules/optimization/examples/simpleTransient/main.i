[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'source'
  num_values = '44'
[]
[Reporters]
  [main]
    type = OptimizationData
  []
[]
[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [params]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/source'
    to_reporters = 'src_values/values'
  []
  [data]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measured_data/objective_value'
    to_reporters = 'OptimizationReporter/objective_value'
  []
[]

[Executioner]
  type = Optimize
  solve_on = none
  tao_solver = taonm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e1'
  verbose = true
[]

[Outputs]
[]
