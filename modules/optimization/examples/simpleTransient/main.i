[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'source'
  num_values = '44'
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
    from_reporters = 'measured_data/misfit_values measured_data/simulation_values'
    to_reporters = 'OptimizationReporter/misfit_values OptimizationReporter/simulation_values'
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
