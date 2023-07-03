[StochasticTools]
[]

[Samplers]
  [vpp]
    type = VectorPostprocessorSampler
    vectors_names = 'csv/year csv/month'
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [csv]
    type = CSVReader
    csv_file = 'example.csv'
  []

  [data]
    type = SamplerData
    sampler = vpp
    sampler_method =get_global_samples
    execute_on = 'FINAL'
  []
[]

[Outputs]
  execute_on = 'FINAL'
  csv = true
[]
