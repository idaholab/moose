[StochasticTools]
  auto_create_executioner = false
[]

[Samplers]
  [sample]
    type = CartesianProduct
    execute_on = PRE_MULTIAPP_SETUP
    linear_space_items = '0 1 3
                          0.0 0.1 5'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = sample
    stochastic_reporter = storage
    from_reporter = 'pp/value constant/int'
  []
[]

[Controls]
  [runner]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Reporters/constant/integer_values
                   Postprocessors/pp/default'
    sampler = sample
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    outputs = "none"
  []
  [stats]
    type = StatisticsReporter
    reporters = 'storage/data:pp:value storage/data:constant:int'
    compute = mean
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.01
[]

[Outputs]
  [out]
    type = JSON
  []
[]
