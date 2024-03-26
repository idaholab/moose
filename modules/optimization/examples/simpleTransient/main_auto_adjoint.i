[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'source'
  num_values = '44'
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [to_forward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/source'
    to_reporters = 'src_values/values'
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measured_data/objective_value adjoint/inner_product'
    to_reporters = 'OptimizationReporter/objective_value OptimizationReporter/grad_source'
  []
[]

[Executioner]
  type = Optimize
  solve_on = none
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gatol -tao_ls_type'
  petsc_options_value = '1e-2 unit'
  verbose = true
[]
