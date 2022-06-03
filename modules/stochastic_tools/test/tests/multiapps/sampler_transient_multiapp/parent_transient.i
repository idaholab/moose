[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform_0]
    type = Uniform
    lower_bound = 0.1
    upper_bound = 0.3
  []
[]

[Samplers]
  [mc]
    type = MonteCarlo
    num_rows = 5
    distributions = 'uniform_0'
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
