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
    multi_app = sub
    sampler = sample
    stochastic_reporter = storage
    from_reporter = 'pp/value constant/str mesh/sidesets'
  []
  [runner]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value'
    to_control = stm
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
