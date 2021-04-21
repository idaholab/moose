[StochasticTools]
[]

[Distributions]
  [mu1]
    type = Normal
    mean = 0.0
    standard_deviation = 0.5
  []
  [mu2]
    type = Normal
    mean = 1
    standard_deviation = 0.5
  []
[]

[Samplers]
  [sample]
    type = AIS
    distributions = 'mu1 mu2'
    execute_on = 'PRE_MULTIAPP_SETUP'
    proposal_std = '0.5 0.5'
    output_limit = 0.45
    num_samples_train = 5
    std_factor = 0.8
    use_absolute_value = true
    seed = 1012
    initial_values = '0.0 1.0'
    inputs_reporter = 'adaptive_MC/left adaptive_MC/right'
    output_reporter = 'adaptive_MC/output_reporter1'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = ../../multiapps/sampler_full_solve_multiapp/sub.i
    sampler = sample
    mode = normal
  []
[]

[Transfers]
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
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
    param_names = 'BCs/left/value BCs/right/value'
  []
[]

[Reporters]
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_names = output_reporter1
    output_values = '0.0'
    inputs_names = 'left right'
    inputs_values = '0.0 0.0'
    sampler = sample
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Outputs]
  file_base = ais_sp_out
  csv = true
  exodus = false
[]
