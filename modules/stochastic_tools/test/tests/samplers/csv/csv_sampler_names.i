[StochasticTools]
[]

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = 'samples.csv'
    column_names = 'a b d'
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
