[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    execute_on = PRE_MULTIAPP_SETUP
    linear_space_items = '0 1 3'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    ignore_solve_not_converge = true
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = sample
    stochastic_reporter = storage
    from_reporter = 'pp/value'
  []
[]

[Controls]
  [runner]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'BCs/left/value'
    sampler = sample
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
