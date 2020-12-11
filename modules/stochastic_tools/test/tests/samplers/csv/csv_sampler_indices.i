[StochasticTools]
[]

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = 'samples.csv'
    column_indices = '0 1 3'
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
