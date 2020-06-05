[StochasticTools]
[]

[Samplers]
  [sample]
    type = PolynomialSampler
    coefficients = '19.80 20.11 20.13'
    x_min = -10
    x_max = 10
    num_rows = 6
    noise = 100
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    disable_gather = true
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  csv = true
[]
