[StochasticTools]
[]

[Samplers]
  [vpp]
    type = VectorPostprocessorSampler
    vectors_names = 'VPP1/year VPP2/month'
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [VPP1]
    type = CSVReader
    csv_file = 'example.csv'
  []

  [VPP2]
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
