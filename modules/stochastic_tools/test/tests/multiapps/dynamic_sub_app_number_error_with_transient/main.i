[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 0.1
    upper_bound = 0.3
  []
[]

[Samplers]
  [mc]
    type = TestDynamicNumberOfSubAppsSampler
    num_rows = 5
    distributions = 'uniform'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[MultiApps]
  [runner]
    type = SamplerTransientMultiApp
    sampler = mc
    input_files = 'sub.i'
  []
[]
