[StochasticTools]
[]

[Distributions]
  [a]
    type = Uniform
    lower_bound = 100
    upper_bound = 200
  []
  [b]
    type = Uniform
    lower_bound = 10
    upper_bound = 20
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'a b'
    num_rows = 10
    seed = 1980
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    sampler_method = get_global_samples
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  csv = true
[]
