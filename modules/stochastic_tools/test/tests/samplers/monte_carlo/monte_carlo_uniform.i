[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 7
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 10
    distributions = 'uniform'
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  csv = true
[]
