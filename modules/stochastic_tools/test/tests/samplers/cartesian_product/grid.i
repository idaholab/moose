[StochasticTools]
[]

[Samplers/sample]
  type = CartesianProduct
  linear_space_items = '10 1.5 3
                        20 1 4
                        130 10 2'
  execute_on = 'initial timestep_end'
[]

[VectorPostprocessors/data]
  type = SamplerData
  sampler = sample
  execute_on = 'initial timestep_end'
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
