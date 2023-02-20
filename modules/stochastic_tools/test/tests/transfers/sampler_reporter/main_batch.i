[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0.0 0.1 10'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-restore
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = sample
    stochastic_reporter = storage
    from_reporter = 'pp/value constant/str'
  []
  [runner]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value'
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    parallel_type = ROOT
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = timestep_end
  []
[]
