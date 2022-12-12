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
    type = AdaptiveImportance
    distributions = 'mu1 mu2'
    proposal_std = '1.0 1.0'
    output_limit = 0.65
    num_samples_train = 30
    num_importance_sampling_steps = 30
    std_factor = 0.9
    initial_values = '-0.103 1.239'
    inputs_reporter = 'adaptive_MC/inputs'
    use_absolute_value = true
    seed = 1012
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_value = constant/reporter_transfer:average:value
    inputs = 'inputs'
    sampler = sample
  []
  [ais_stats]
    type = AdaptiveImportanceStats
    output_value = constant/reporter_transfer:average:value
    sampler = sample
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  [out]
    type = JSON
  []
[]
