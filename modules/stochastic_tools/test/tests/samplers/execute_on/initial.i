[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = -42
    upper_bound = 42
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 10
    distributions = 'uniform'
    execute_on = 'initial' # Create random numbers on initial only, they remain the same with time.
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  csv = true
[]
