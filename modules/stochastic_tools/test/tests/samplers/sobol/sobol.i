[StochasticTools]
[]

[Distributions]
  [d0]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
  [d1]
    type = Uniform
    lower_bound = 10
    upper_bound = 11
  []
  [d2]
    type = Uniform
    lower_bound = 100
    upper_bound = 101
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    distributions = 'd0 d1 d2'
    num_rows = 4
    seed = 2011
  []
  [resample]
    type = MonteCarlo
    distributions = 'd0 d1 d2'
    num_rows = 4
    seed = 2013
  []
  [sobol]
    type = Sobol
    sampler_a = sample
    sampler_b = resample
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sobol
    execute_on = 'initial'
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
