[StochasticTools]
[]

[Distributions/dist]
  type = Uniform
[]

[Samplers/sample]
  type = MonteCarlo
  distributions = 'dist'
  num_rows = 10
[]

[VectorPostprocessors/data]
  type = SamplerData
  sampler = sample
  sampler_method = get_next_local_row
[]

[Outputs]
  execute_on = timestep_end
  csv = true
[]
