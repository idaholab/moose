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
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = ADJOINT
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
    from_reporters = 'measured_data/misfit_values measured_data/objective_value'
    to_reporters = 'main/misfit_values OptimizationReporter/objective_value'
  []
  [to_adjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/source main/misfit_values'
    to_reporters = 'src_values/values measured_data/misfit_values'
  []
  [from_adjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint/inner_product'
    to_reporters = 'OptimizationReporter/grad_source'
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

[Postprocessors]
  [elapsed]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  []
[]

[Outputs]
  [pgraph]
    type = PerfGraphOutput
    level = 1
  []
[]
