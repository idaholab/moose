[StochasticTools]
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

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
  []
[]
