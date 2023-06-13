[StochasticTools]
[]

[Samplers]
  [sample]
    type = Cartesian1D
    linear_space_items = '10 1.5 3
                          20 1 4
                          130 10 2'
    nominal_values = '10.1 20.1 130.1'
  []
[]

[VectorPostprocessors/data]
  type = SamplerData
  sampler = sample
  execute_on = initial
[]

[Outputs]
  execute_on = initial
  csv = true
[]
