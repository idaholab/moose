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
    proposal_std = '0.15 0.15'
    output_limit = 0.45
    num_samples_train = 30
    std_factor = 0.8
    use_absolute_value = true
    seed = 1012
    initial_values = '-0.10329808102501603 1.2396280668056123'
    inputs_reporter = 'adaptive_MC/inputs'
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
    to_control = 'stochastic'
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
[]

[Executioner]
  type = Transient
  num_steps = 40
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
