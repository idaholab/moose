[StochasticTools]
  # auto_create_executioner = false
[]

[Distributions]
  [mu1]
    type = Normal
    mean = 0.3
    standard_deviation = 0.045
  []
  [mu2]
    type = Normal
    mean = 9
    standard_deviation = 1.35
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = 'PRE_MULTIAPP_SETUP TIMESTEP_END'
    seed = 100
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = ../../../../examples/parameter_study/nonlin_diff_react/nonlin_diff_react_sub.i
    sampler = sample
    mode = normal
    reset_subapp_transient = false
  []
[]

[Transfers]
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
    check_multiapp_execute_on = true
  []
  [data]
    type = MultiAppReporterTransfer
    to_reporters = 'results_reporter/results_reporter1'
    from_reporters = 'average/value'
    direction = from_multiapp
    multi_app = sub
    subapp_index = 0
  []
[]

[Controls]
  [cmdline]
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = sample
    param_names = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
  []
[]

[Reporters]
  [results_reporter]
    type = ConstantReporter
    real_names = results_reporter1
    real_values = '0.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  file_base = 'main_MC_normal_true'
  csv = true
  exodus = false
[]
