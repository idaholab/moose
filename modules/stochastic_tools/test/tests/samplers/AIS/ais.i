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
    type = AIS
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = PRE_MULTIAPP_SETUP
    proposal_std = '0.5 0.5'
    output_limit = 0.19
    num_samples_train = 250
    std_factor = 0.8
    use_absolute_value = true
    seed = 1012
    seeds = '0.302784298 13.36093888'
    inputs_reporter = 'adaptive_MC/mu1 adaptive_MC/mu2'
    output_reporter = 'adaptive_MC/output_reporter1'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub1.i
    sampler = sample
    mode = batch-reset
  []
[]

[Transfers]
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [data]
    type = MultiAppReporterTransfer
    to_reporters = 'adaptive_MC/output_reporter1'
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
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_names = output_reporter1
    output_values = '0.0'
    inputs_names = 'mu1 mu2'
    inputs_values = '0.0 0.0'
    sampler = sample
  []
[]

[Executioner]
  type = Transient
  num_steps = 1000
[]

[Outputs]
  csv = true
  exodus = false
[]
