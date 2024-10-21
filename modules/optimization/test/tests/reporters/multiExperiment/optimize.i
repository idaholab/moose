[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  parameter_names = 'vals'
  num_values = '2'
  objective_name = obj_value
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnktr
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-8 '
  verbose = true
[]

[MultiApps]
  [forward_sampler]
    type = FullSolveMultiApp
    input_files = sampler_subapp.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward_sampler
    from_reporters = 'OptimizationReporter/vals'
    to_reporters = 'parameters/vals'
  []

  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward_sampler
    from_reporters = 'obj_sum/value
                      grad_sum/row_sum'
    to_reporters = 'OptimizationReporter/obj_value
                    OptimizationReporter/grad_vals'
  []
[]

[Outputs]
  csv = true
[]
