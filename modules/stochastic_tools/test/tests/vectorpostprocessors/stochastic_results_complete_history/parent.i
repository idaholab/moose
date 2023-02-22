[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform_left]
    type = Uniform
    lower_bound = 0
    upper_bound = 0.5
  []
  [uniform_right]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 3
    distributions = 'uniform_left uniform_right'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[MultiApps]
  [sub]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Transfers]
  [runner]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
  []
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = storage
    from_postprocessor = avg
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    parallel_type = REPLICATED
    contains_complete_history = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  [out]
    type = CSV
  []
[]
