[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 10
  []
[]

[Samplers]
  [dynamic]
    type = TestDynamicNumberOfSubAppsSampler
    num_rows = 5
    distributions = 'uniform'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[VectorPostprocessors]
  [sample]
    type = SamplerData
    sampler = dynamic
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  [out]
    type = JSON
    vectorpostprocessors_as_reporters = true
  []
[]
