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
    type = ParallelSubsetSimulation
    distributions = 'mu1 mu2'
    output_reporter = 'constant/reporter_transfer:average:value'
    inputs_reporter = 'adaptive_MC/inputs'
    num_samplessub = 20
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
    multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    multi_app = sub
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
  num_steps = 20
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
