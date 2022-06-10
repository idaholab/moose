[StochasticTools]
[]

[Distributions/dist]
  type = Uniform
  lower_bound = 0
  upper_bound = 1
[]

[Samplers]
  [morris]
    type = MorrisSampler
    distributions = 'dist dist dist'
    trajectories = 4
    levels = 6
  []
[]

[VectorPostprocessors/data]
  type = SamplerData
  sampler = morris
  execute_on = 'initial timestep_end'
[]

[Outputs]
  csv = true
[]
