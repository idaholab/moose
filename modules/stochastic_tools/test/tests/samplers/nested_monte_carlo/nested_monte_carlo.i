[StochasticTools]
[]

[Distributions/uniform]
  type = Uniform
  lower_bound = 0
  upper_bound = 1
[]

[Samplers]
  [nested_mc]
    type = NestedMonteCarlo
    num_rows = '10 5 2'
    distributions = 'uniform uniform uniform; uniform uniform; uniform'
  []
[]

[VectorPostprocessors/data]
  type = SamplerData
  sampler = nested_mc
[]

[Outputs]
  execute_on = timestep_end
  csv = true
[]
